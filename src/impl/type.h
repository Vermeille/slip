#pragma once

#include <cassert>
#include <map>
#include <memory>
#include <set>
#include <sstream>

namespace slip {
namespace experimental {

class Namer {
    int x_;

   public:
    Namer() : x_(0) {}

    int NewName() {
        ++x_;
        return x_;
    }
};

struct TypeVisitor;
struct ConstTypeVisitor;

struct Type {
    virtual void Accept(TypeVisitor&) = 0;
    virtual void Accept(ConstTypeVisitor&) const = 0;
};

struct TypeVar : public Type {
    int id_;

    TypeVar(int id) : id_(id) {}

    virtual void Accept(TypeVisitor&) override;
    virtual void Accept(ConstTypeVisitor&) const override;
};

struct Arrow : public Type {
    std::unique_ptr<Type> lhs_;
    std::unique_ptr<Type> rhs_;

    Arrow() = default;

    template <class T, class U>
    Arrow(T&& x, U&& y)
        : lhs_(new T(std::forward<T>(x))), rhs_(new U(std::forward<U>(y))) {}

    Arrow(std::unique_ptr<Type>&& lhs, std::unique_ptr<Type> rhs)
        : lhs_(std::move(lhs)), rhs_(std::move(rhs)) {}

    virtual void Accept(TypeVisitor&) override;
    virtual void Accept(ConstTypeVisitor&) const override;
};

struct ConstType : public Type {
    std::string name_;

    ConstType(std::string str) : name_(std::move(str)) {}

    virtual void Accept(TypeVisitor&) override;
    virtual void Accept(ConstTypeVisitor&) const override;
};

struct TypeVisitor {
    virtual void Visit(Type& tv) { tv.Accept(*this); }
    virtual void Visit(TypeVar& tv) { tv.Accept(*this); }
    virtual void Visit(ConstType& t) { t.Accept(*this); }
    virtual void Visit(Arrow& arr) {
        arr.lhs_->Accept(*this);
        arr.rhs_->Accept(*this);
    }
};

struct ConstTypeVisitor {
    virtual void Visit(const Type& tv) { tv.Accept(*this); }
    virtual void Visit(const TypeVar& tv) { tv.Accept(*this); }
    virtual void Visit(const ConstType& t) { t.Accept(*this); }
    virtual void Visit(const Arrow& arr) {
        arr.lhs_->Accept(*this);
        arr.rhs_->Accept(*this);
    }
};

void ConstType::Accept(TypeVisitor& vis) { vis.Visit(*this); }
void ConstType::Accept(ConstTypeVisitor& vis) const { vis.Visit(*this); }
void TypeVar::Accept(TypeVisitor& vis) { vis.Visit(*this); }
void TypeVar::Accept(ConstTypeVisitor& vis) const { vis.Visit(*this); }
void Arrow::Accept(TypeVisitor& vis) { vis.Visit(*this); }
void Arrow::Accept(ConstTypeVisitor& vis) const { vis.Visit(*this); }

using Substitutions = std::map<int, std::unique_ptr<Type>>;

struct TypeShowVisitor : public ConstTypeVisitor {
    enum class From { Left, Right };
    From arg_;
    std::string res_;

    TypeShowVisitor() : arg_(From::Left) {}

    virtual void Visit(const TypeVar& tv) override {
        res_ += "t" + std::to_string(tv.id_);
    }

    virtual void Visit(const ConstType& t) override { res_ += t.name_; }

    virtual void Visit(const Arrow& arr) override {
        From from = arg_;
        if (from == From::Right) {
            res_ += "(";
        }
        arg_ = From::Right;
        arr.lhs_->Accept(*this);

        res_ += " -> ";

        arg_ = From::Left;
        arr.rhs_->Accept(*this);
        if (from == From::Right) {
            res_ += ")";
        }
    }
    const std::string& result() const { return res_; }
};

struct FindVarsVisitor : public ConstTypeVisitor {
    std::set<int> vars_;
    virtual void Visit(const TypeVar& tv) override { vars_.insert(tv.id_); }
    virtual void Visit(const ConstType&) override {}
    const std::set<int>& result() const { return vars_; }
};

struct ClonerVisitor : public ConstTypeVisitor {
    std::unique_ptr<Type> ret_;

    virtual void Visit(const TypeVar& tv) override {
        ret_ = std::make_unique<TypeVar>(tv.id_);
    }

    virtual void Visit(const ConstType& ct) override {
        ret_ = std::make_unique<ConstType>(ct.name_);
    }

    virtual void Visit(const Arrow& arr) override {
        auto ptr = std::make_unique<Arrow>();

        arr.lhs_->Accept(*this);
        ptr->lhs_ = std::move(ret_);

        arr.rhs_->Accept(*this);
        ptr->rhs_ = std::move(ret_);

        ret_ = std::move(ptr);
    }

    std::unique_ptr<Type> result() { return std::move(ret_); }
};

struct Prototype {
    std::set<int> vars_;
    std::unique_ptr<Type> type_;

    Prototype(const Prototype& o) : vars_(o.vars_) {
        ClonerVisitor cv;
        o.type_->Accept(cv);
        type_ = cv.result();
    }

    Prototype(std::unique_ptr<Type>&& ty) : type_(std::move(ty)) {
        FindVarsVisitor vis;
        type_->Accept(vis);
        vars_ = vis.result();
    }

    Prototype(Arrow&& x)
        : Prototype(std::make_unique<Arrow>(std::forward<Arrow>(x))) {}

    std::string Show() const {
        std::string forall;
        if (!vars_.empty()) {
            forall = "forall";
            for (int i : vars_) {
                forall += " t" + std::to_string(i);
            }
            forall += ". ";
        }
        TypeShowVisitor tsv;
        type_->Accept(tsv);
        return forall + tsv.result();
    }
};

}  // namespace experimental
}  // namespace slip

#pragma once

#include <cassert>
#include <map>
#include <memory>
#include <set>
#include <sstream>

#include "parcxx/src/parcxx.h"

namespace slip {

class Namer {
    int x_;

   public:
    Namer() : x_(0) {}

    int NewName() { return x_++; }
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

    Arrow(std::unique_ptr<Type> lhs, std::unique_ptr<Type> rhs)
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

std::string Show(const Type& ty) {
    TypeShowVisitor tsv;
    ty.Accept(tsv);
    return tsv.result();
}

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

std::unique_ptr<Type> Clone(const Type& x) {
    ClonerVisitor cv;
    x.Accept(cv);
    return cv.result();
}

using Substitutions = std::map<int, std::unique_ptr<Type>>;

struct UnifyVisitor : public ConstTypeVisitor {
    Substitutions subs_;
    const Type* ptr_;

    UnifyVisitor(const Type* ptr) : ptr_(ptr) {}

    void Bind(const TypeVar& tv, const Type& ty) {
        auto inserted = subs_.insert(std::make_pair(tv.id_, Clone(ty)));
        if (!inserted.second) {
            throw std::runtime_error("t" + std::to_string(tv.id_) +
                                     " was already bound to " +
                                     Show(*inserted.first->second));
        }
    }

    virtual void Visit(const TypeVar& tv) override { Bind(tv, *ptr_); }

    virtual void Visit(const ConstType& ct) override {
        auto as_const_type = dynamic_cast<const ConstType*>(ptr_);
        if (as_const_type) {
            if (ct.name_ == as_const_type->name_) {
                return;
            }

            throw std::runtime_error("Can't unify " + Show(ct) + " and " +
                                     Show(*ptr_));
        }
        auto as_type_var = dynamic_cast<const TypeVar*>(ptr_);
        if (as_type_var) {
            Bind(*as_type_var, ct);
            return;
        }
        throw std::runtime_error("Can't unify " + Show(ct) + " and " +
                                 Show(*ptr_));
    }

    virtual void Visit(const Arrow& ct) override {
        auto as_arrow = dynamic_cast<const Arrow*>(ptr_);
        if (!as_arrow) {
            throw std::runtime_error("Can't unify " + Show(ct) + " and " +
                                     Show(*ptr_));
        }
        ptr_ = as_arrow->lhs_.get();
        ct.lhs_->Accept(*this);

        ptr_ = as_arrow->rhs_.get();
        ct.rhs_->Accept(*this);
    }

    Substitutions result() { return std::move(subs_); }
};

Substitutions Bind(const Type& lhs, const Type& rhs) {
    UnifyVisitor uvis(&rhs);
    lhs.Accept(uvis);
    return uvis.result();
}

struct ApplySubstitution : public TypeVisitor {
    const Substitutions& subs_;

    std::unique_ptr<Type>* parent_;

    ApplySubstitution(const Substitutions& subs, std::unique_ptr<Type>* root)
        : subs_(subs), parent_(root) {}

    virtual void Visit(TypeVar& tv) override {
        auto found = subs_.find(tv.id_);
        if (found == subs_.end()) {
            return;
        }
        *parent_ = Clone(*found->second);
    }

    virtual void Visit(ConstType&) override {}

    virtual void Visit(Arrow& arr) override {
        parent_ = &arr.lhs_;
        arr.lhs_->Accept(*this);

        parent_ = &arr.rhs_;
        arr.rhs_->Accept(*this);
    }
};

void Substitute(const Substitutions& subs, std::unique_ptr<Type>& ty) {
    ApplySubstitution vis(subs, &ty);
    ty->Accept(vis);
}

struct Prototype {
    std::set<int> vars_;
    std::unique_ptr<Type> type_;

    Prototype() = default;

    Prototype(Prototype&& o) = default;

    Prototype(const Prototype& o) : vars_(o.vars_) { type_ = Clone(*o.type_); }

    Prototype(std::unique_ptr<Type> ty) : type_(std::move(ty)) {
        FindVarsVisitor vis;
        type_->Accept(vis);
        vars_ = vis.result();
    }

    Prototype(Arrow&& x)
        : Prototype(std::make_unique<Arrow>(std::forward<Arrow>(x))) {}
    Prototype(TypeVar&& x) : type_(new TypeVar(x)) {}
    Prototype(ConstType&& x) : type_(new ConstType(x)) {}

    Prototype& operator=(const Prototype& o) {
        vars_ = o.vars_;
        type_ = Clone(*o.type_);
        return *this;
    }

    Prototype& operator=(Prototype&& o) = default;

    std::string Show() const {
        std::string forall;
        if (!vars_.empty()) {
            forall = "forall";
            for (int i : vars_) {
                forall += " t" + std::to_string(i);
            }
            forall += ". ";
        }
        return forall + ::slip::Show(*type_);
    }

    void Instantiate(Namer& namer) {
        Substitutions subs;
        std::set<int> renamed_vars;
        for (int x : vars_) {
            int name = namer.NewName();
            subs[x] = std::make_unique<TypeVar>(name);
            renamed_vars.insert(name);
        }
        vars_.swap(renamed_vars);
        Substitute(subs, type_);
    }

    Prototype Substitue(int ty, const Prototype& pro) const {
        Namer namer;

        Prototype fun = *this;
        Prototype args = pro;

        fun.Instantiate(namer);
        args.Instantiate(namer);

        Substitutions subs;
        subs[ty] = Clone(*args.type_);
        slip::Substitute(subs, fun.type_);
        return Prototype(std::move(fun.type_));
    }

    Prototype Apply(const Prototype& pro) const {
        Namer namer;

        Prototype fun = *this;
        Prototype args = pro;

        fun.Instantiate(namer);
        args.Instantiate(namer);

        auto as_fun = dynamic_cast<Arrow*>(fun.type_.get());
        if (!as_fun) {
            throw std::runtime_error(fun.Show() + " is of non-type function");
        }

        auto subs = Bind(*as_fun->lhs_, *args.type_);
        Substitute(subs, as_fun->rhs_);

        return Prototype(std::move(as_fun->rhs_));
    }
};

int LowerCaseIdToNbr(const std::string& str) {
    int res = 0;
    for (char c : str) {
        res = res * 26 + (c - 'a');
    }
    return res;
}

template <class P>
auto Tok2(P p) {
    return skip_while(parser_pred(parse_char(), isblank)) >> p;
}

Parser<std::unique_ptr<Type>> ParseType() {
    auto name_to_type = [](const std::string& name) -> std::unique_ptr<Type> {
        if (islower(name[0])) {
            return std::make_unique<TypeVar>(LowerCaseIdToNbr(name));
        }
        return std::make_unique<ConstType>(name);
    };

    auto atom_type = Tok2(parse_word()) % name_to_type;
    auto paren = [](auto&& fun) {
        return Tok2(parse_char('(')) >> fun << Tok2(parse_char(')'));
    };
    auto arrow = Tok2(parse_char('-')) >> parse_char('>');
    Parser<std::unique_ptr<Type>> fun =
        ((atom_type | paren(recursion(fun))) & !(arrow >> recursion(fun))) %
        [](auto&& x) -> std::unique_ptr<Type> {
        if (!x.second) {
            return std::move(x.first);
        }
        return std::make_unique<Arrow>(std::move(x.first),
                                       std::move(*x.second));
    };
    return fun;
}

auto ParseType(const std::string& input) {
    static const auto parser = ParseType();
    auto res = parser(input.begin(), input.end());
    if (!res) {
        return Prototype();
    }
    return Prototype(std::move(res->first));
}

}  // namespace slip

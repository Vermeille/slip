#pragma once

#include <cassert>
#include <map>
#include <memory>
#include <set>
#include <sstream>

#include "parcxx/src/parcxx.h"

#include <boost/variant.hpp>

namespace slip {

class Namer {
    int x_;

   public:
    Namer() : x_(0) {}

    int NewName() { return x_++; }
};

class TypeVar {
   public:
    TypeVar(int id) : id_(id) {}
    TypeVar() = default;

    int id() const { return id_; }

   private:
    int id_;
};

struct ConstType {
   public:
    ConstType(std::string str) : name_(std::move(str)) {}

    const std::string& name() const { return name_; }

   private:
    std::string name_;
};

class Arrow;
using Type =
    boost::variant<TypeVar, ConstType, boost::recursive_wrapper<Arrow>>;

class Arrow {
   public:
    Arrow() = default;

    Arrow(Type lhs, Type rhs) : lhs_(std::move(lhs)), rhs_(std::move(rhs)) {}

    Type& lhs() { return lhs_; }
    Type& rhs() { return rhs_; }
    const Type& lhs() const { return lhs_; }
    const Type& rhs() const { return rhs_; }

   private:
    Type lhs_;
    Type rhs_;
};

std::string IdToLetters(int id) {
    std::string res;
    res.insert(res.begin(), id % 26 + 'a');
    id /= 26;
    while (id) {
        res.insert(res.begin(), id % 26 + 'a');
        id /= 26;
    }
    return res;
}

class TypeShowVisitor : public boost::static_visitor<void> {
   public:
    TypeShowVisitor() : arg_(From::Left) {}

    void operator()(const TypeVar& tv) { res_ += IdToLetters(tv.id()); }

    void operator()(const ConstType& t) { res_ += t.name(); }

    void operator()(const Arrow& arr) {
        From from = arg_;
        if (from == From::Right) {
            res_ += "(";
        }
        arg_ = From::Right;
        boost::apply_visitor(*this, arr.lhs());

        res_ += " -> ";

        arg_ = From::Left;
        boost::apply_visitor(*this, arr.rhs());
        if (from == From::Right) {
            res_ += ")";
        }
    }
    const std::string& result() const { return res_; }

   private:
    enum class From { Left, Right };
    From arg_;
    std::string res_;
};

std::string Show(const Type& ty) {
    TypeShowVisitor tsv;
    boost::apply_visitor(tsv, ty);
    return tsv.result();
}

struct FindVarsVisitor : public boost::static_visitor<void> {
    std::set<int> vars_;
    void operator()(const TypeVar& tv) { vars_.insert(tv.id()); }
    void operator()(const ConstType&) {}
    void operator()(const Arrow& r) {
        boost::apply_visitor(*this, r.lhs());
        boost::apply_visitor(*this, r.rhs());
    }
    const std::set<int>& result() const { return vars_; }
};

using Substitutions = std::map<int, Type>;

struct UnifyVisitor : public boost::static_visitor<void> {
    Substitutions subs_;
    const Type* ptr_;

    UnifyVisitor(const Type* ptr) : ptr_(ptr) {}

    void Bind(const TypeVar& tv, const Type& ty) {
        auto inserted = subs_.insert(std::make_pair(tv.id(), ty));
        if (!inserted.second) {
            throw std::runtime_error("t" + std::to_string(tv.id()) +
                                     " was already bound to " +
                                     Show(inserted.first->second));
        }
    }

    void operator()(const TypeVar& tv) { Bind(tv, *ptr_); }

    void operator()(const ConstType& ct) {
        auto as_const_type = boost::get<ConstType>(ptr_);
        if (as_const_type) {
            if (ct.name() == as_const_type->name()) {
                return;
            }

            throw std::runtime_error("Can't unify " + Show(ct) + " and " +
                                     Show(*ptr_));
        }
        auto as_type_var = boost::get<TypeVar>(ptr_);
        if (as_type_var) {
            Bind(*as_type_var, ct);
            return;
        }
        throw std::runtime_error("Can't unify " + Show(ct) + " and " +
                                 Show(*ptr_));
    }

    void operator()(const Arrow& ct) {
        auto as_arrow = boost::get<Arrow>(ptr_);
        if (!as_arrow) {
            throw std::runtime_error("Can't unify " + Show(ct) + " and " +
                                     Show(*ptr_));
        }
        ptr_ = &as_arrow->lhs();
        boost::apply_visitor(*this, ct.lhs());

        ptr_ = &as_arrow->rhs();
        boost::apply_visitor(*this, ct.rhs());
    }

    Substitutions result() { return std::move(subs_); }
};

Substitutions Bind(const Type& lhs, const Type& rhs) {
    UnifyVisitor uvis(&rhs);
    boost::apply_visitor(uvis, lhs);
    return uvis.result();
}

class ApplySubstitution : public boost::static_visitor<> {
   public:
    ApplySubstitution(const Substitutions& subs, Type* root)
        : subs_(subs), parent_(root) {}

    void operator()(TypeVar& tv) {
        auto found = subs_.find(tv.id());
        if (found == subs_.end()) {
            return;
        }
        *parent_ = found->second;
    }

    void operator()(ConstType&) {}

    void operator()(Arrow& arr) {
        parent_ = &arr.lhs();
        boost::apply_visitor(*this, arr.lhs());

        parent_ = &arr.rhs();
        boost::apply_visitor(*this, arr.rhs());
    }

   private:
    const Substitutions& subs_;
    Type* parent_;
};

void Substitute(const Substitutions& subs, Type& ty) {
    ApplySubstitution vis(subs, &ty);
    boost::apply_visitor(vis, ty);
}

int GetArity(const Type& x) {
    struct ArityVisitor : public boost::static_visitor<int> {
        int operator()(const ConstType&) const { return 0; }
        int operator()(const TypeVar&) const { return 0; }
        int operator()(const Arrow& a) const {
            return 1 + boost::apply_visitor(*this, a.rhs());
        }
    } vis;
    return boost::apply_visitor(vis, x);
}

class Prototype {
   public:
    Prototype() = default;

    Prototype(Prototype&& o) = default;

    Prototype(const Prototype& o)
        : vars_(o.vars_), type_(o.type_), arity_(o.arity_) {}

    Prototype(Type ty) : type_(std::move(ty)), arity_(GetArity(type_)) {
        FindVarsVisitor vis;
        boost::apply_visitor(vis, type_);
        vars_ = vis.result();
    }

    Prototype& operator=(const Prototype& o) = default;
    Prototype& operator=(Prototype&& o) = default;

    std::string Show() const {
        std::string forall;
        if (!vars_.empty()) {
            forall = "forall";
            for (int i : vars_) {
                forall += " " + IdToLetters(i);
            }
            forall += ". ";
        }
        return forall + ::slip::Show(type_);
    }

    void Instantiate(Namer& namer) {
        Substitutions subs;
        std::set<int> renamed_vars;
        for (int x : vars_) {
            int name = namer.NewName();
            subs[x] = TypeVar(name);
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
        subs[ty] = args.type_;
        slip::Substitute(subs, fun.type_);
        return Prototype(std::move(fun.type_));
    }

    Prototype Apply(const Prototype& pro) const {
        Namer namer;

        Prototype fun = *this;
        Prototype args = pro;

        fun.Instantiate(namer);
        args.Instantiate(namer);

        auto as_fun = boost::get<Arrow>(&fun.type_);
        if (!as_fun) {
            throw std::runtime_error(fun.Show() + " is of non-type function");
        }

        auto subs = Bind(as_fun->lhs(), args.type_);
        slip::Substitute(subs, as_fun->rhs());

        return Prototype(std::move(as_fun->rhs()));
    }

    bool IsFunction() const { return boost::get<Arrow>(&type_) != nullptr; }

    int arity() const { return arity_; }

   private:
    std::set<int> vars_;
    Type type_;
    int arity_;
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

Parser<Type> ParseType() {
    auto name_to_type = [](const std::string& name) -> Type {
        if (islower(name[0])) {
            return TypeVar(LowerCaseIdToNbr(name));
        }
        return ConstType(name);
    };

    auto atom_type = Tok2(parse_word()) % name_to_type;
    auto paren = [](auto&& fun) {
        return Tok2(parse_char('(')) >> fun << Tok2(parse_char(')'));
    };
    auto arrow = Tok2(parse_char('-')) >> parse_char('>');
    Parser<Type> fun =
        ((atom_type | paren(recursion(fun))) & !(arrow >> recursion(fun))) %
        [](auto&& x) -> Type {
        if (!x.second) {
            return std::move(x.first);
        }
        return Arrow(std::move(x.first), std::move(*x.second));
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

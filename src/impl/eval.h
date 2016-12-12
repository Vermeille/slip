#pragma once

#include <string>
#include <type_traits>

#include "ast.h"
#include "closure.h"
#include "context.h"
#include "polymorphic.h"

namespace slip {
void ApplyOnArgs(Closure& fun, const List& xs, Context& ctx) {
    for (size_t i = 1; i < xs.size(); ++i) {
        fun.Apply(xs[i], ctx);
    }
}

template <class T>
T Eval(const Val& x, Context& ctx);
template <>
std::string Eval<std::string>(const Val& v, Context& ctx);

template <class T>
T Eval(const Val& x, Context& ctx) {
    if (const List* i = boost::get<List>(&x)) {
        Closure fun = Eval<Closure>((*i)[0], ctx);
        ApplyOnArgs(fun, *i, ctx);
        return fun.GetResult<T>();
    }
    throw std::runtime_error("expected a " + GetTypeId<T>::type() +
                             " expression");
}

template <>
Closure Eval<Closure>(const Val& x, Context& ctx) {
    if (const Atom* i = boost::get<Atom>(&x)) {
        std::string funname = Eval<std::string>(*i, ctx);
        auto fun = ctx.Find(funname);
        if (!fun) {
            throw std::runtime_error("no such function: " + funname);
        }
        return fun->GetClosure();
    } else if (const List* l = boost::get<List>(&x)) {
        Closure fun = Eval<Closure>((*l)[0], ctx);
        ApplyOnArgs(fun, *l, ctx);
        return fun;
    }
    throw std::runtime_error("Expected a closure");
}

template <>
std::string Eval<std::string>(const Val& v, Context& ctx) {
    struct EvalVisitor : public boost::static_visitor<std::string> {
        Context& ctx_;
        EvalVisitor(Context& ctx) : ctx_(ctx) {}

        std::string operator()(const Atom& a) const { return a.val(); }
        std::string operator()(const Str& s) const { return s.val(); }
        std::string operator()(const List& l) const {
            Closure fun = Eval<Closure>(l[0], ctx_);
            ApplyOnArgs(fun, l, ctx_);
            return fun.GetResult<std::string>();
        }
        std::string operator()(const Bool&) const {
            throw std::runtime_error("expected a str expression");
        }
        std::string operator()(const Int&) const {
            throw std::runtime_error("expected a str expression");
        }
    } vis(ctx);
    return boost::apply_visitor(vis, v);
}

template <>
int Eval<int>(const Val& v, Context& ctx) {
    if (const Int* i = boost::get<Int>(&v)) {
        return i->val();
    } else if (const List* i = boost::get<List>(&v)) {
        Closure fun = Eval<Closure>((*i)[0], ctx);
        ApplyOnArgs(fun, *i, ctx);
        return fun.GetResult<int>();
    }
    throw std::runtime_error("expected an int expression");
}

template <>
bool Eval<bool>(const Val& v, Context& ctx) {
    if (const Bool* i = boost::get<Bool>(&v)) {
        return i->val();
    } else if (const List* i = boost::get<List>(&v)) {
        Closure fun = Eval<Closure>((*i)[0], ctx);
        return fun.GetResult<bool>();
    }
    throw std::runtime_error("expected an int expression");
}

template <>
Polymorphic Eval<Polymorphic>(const Val& v, Context& ctx) {
    struct EvalVis : public boost::static_visitor<Polymorphic> {
        Context& ctx_;
        EvalVis(Context& ctx) : ctx_(ctx) {}

        Polymorphic operator()(const Int& i) const { return i.val(); }
        Polymorphic operator()(const Atom& a) const { return a.val(); }
        Polymorphic operator()(const Str& s) const { return s.val(); }
        Polymorphic operator()(const Bool& b) const { return b.val(); }

        Polymorphic operator()(const List& i) const {
            Closure fun = Eval<Closure>(i[0], ctx_);
            ApplyOnArgs(fun, i, ctx_);
            return fun.GetResult();
        }
    } vis(ctx);
    return boost::apply_visitor(vis, v);
}
}  // namespace slip

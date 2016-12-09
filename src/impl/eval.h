#pragma once

#include <string>
#include <type_traits>

#include "ast.h"
#include "context.h"
#include "polymorphic.h"

namespace slip {
template <class T>
T Eval(const Val& x, Context& ctx) {
    if (const List* i = boost::get<List>(&x)) {
        std::string funname = Eval<std::string>((*i)[0], ctx);
        auto fun = ctx.Find(funname);
        if (!fun) {
            throw std::runtime_error("no such function: " + funname);
        }
        return fun->Call<std::decay_t<T>>(*i, ctx);
    }
    throw std::runtime_error("expected a " + GetTypeId<T>::type() +
                             " expression");
}

template <>
std::string Eval<std::string>(const Val& v, Context& ctx) {
    struct EvalVisitor : public boost::static_visitor<std::string> {
        Context& ctx_;
        EvalVisitor(Context& ctx) : ctx_(ctx) {}

        std::string operator()(const Atom& a) const { return a.val(); }
        std::string operator()(const Str& s) const { return s.val(); }
        std::string operator()(const List& l) const {
            std::string funname = Eval<std::string>(l[0], ctx_);
            auto fun = ctx_.Find(funname);
            if (!fun) {
                throw std::runtime_error("no such function: " + funname);
            }
            return fun->Call<std::string>(l, ctx_);
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
        std::string funname = Eval<std::string>((*i)[0], ctx);
        auto fun = ctx.Find(funname);
        if (!fun) {
            throw std::runtime_error("no such function: " + funname);
        }
        return fun->Call<int>(*i, ctx);
    }
    throw std::runtime_error("expected an int expression");
}

template <>
bool Eval<bool>(const Val& v, Context& ctx) {
    if (const Bool* i = boost::get<Bool>(&v)) {
        return i->val();
    } else if (const List* i = boost::get<List>(&v)) {
        std::string funname = Eval<std::string>((*i)[0], ctx);
        auto fun = ctx.Find(funname);
        if (!fun) {
            throw std::runtime_error("no such function: " + funname);
        }
        return fun->Call<bool>(*i, ctx);
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
            std::string funname = Eval<std::string>(i[0], ctx_);
            auto fun = ctx_.Find(funname);
            if (!fun) {
                throw std::runtime_error("no such function: " + funname);
            }
            return fun->Call<Polymorphic>(i, ctx_);
        }
    } vis(ctx);
    // throw std::runtime_error("expected any expression");
    return boost::apply_visitor(vis, v);
}
}  // namespace slip

#pragma once

#include <string>
#include <type_traits>

#include <boost/any.hpp>

#include "ast.h"
#include "context.h"

namespace slip {
template <class T>
T Eval(const slip::Val& x, Context& ctx) {
    if (const slip::List* i = dynamic_cast<const slip::List*>(&x)) {
        std::string funname = Eval<std::string>(*(*i)[0], ctx);
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
std::string Eval<std::string>(const slip::Val& v, Context& ctx) {
    if (const slip::Atom* i = dynamic_cast<const slip::Atom*>(&v)) {
        return i->val();
    } else if (const slip::Str* i = dynamic_cast<const slip::Str*>(&v)) {
        return i->val();
    } else if (const slip::List* i = dynamic_cast<const slip::List*>(&v)) {
        std::string funname = Eval<std::string>(*(*i)[0], ctx);
        auto fun = ctx.Find(funname);
        if (!fun) {
            throw std::runtime_error("no such function: " + funname);
        }
        return fun->Call<std::string>(*i, ctx);
    }
    throw std::runtime_error("expected a str expression");
}

template <>
int Eval<int>(const slip::Val& v, Context& ctx) {
    if (const slip::Int* i = dynamic_cast<const slip::Int*>(&v)) {
        return i->val();
    } else if (const slip::List* i = dynamic_cast<const slip::List*>(&v)) {
        std::string funname = Eval<std::string>(*(*i)[0], ctx);
        auto fun = ctx.Find(funname);
        if (!fun) {
            throw std::runtime_error("no such function: " + funname);
        }
        return fun->Call<int>(*i, ctx);
    }
    throw std::runtime_error("expected an int expression");
}

template <>
boost::any Eval<boost::any>(const slip::Val& v, Context& ctx) {
    if (const slip::Int* i = dynamic_cast<const slip::Int*>(&v)) {
        return i->val();
    } else if (const slip::Atom* i = dynamic_cast<const slip::Atom*>(&v)) {
        return i->val();
    } else if (const slip::Str* i = dynamic_cast<const slip::Str*>(&v)) {
        return i->val();
    } else if (const slip::List* i = dynamic_cast<const slip::List*>(&v)) {
        std::string funname = Eval<std::string>(*(*i)[0], ctx);
        auto fun = ctx.Find(funname);
        if (!fun) {
            throw std::runtime_error("no such function: " + funname);
        }
        return fun->Call<boost::any>(*i, ctx);
    }
    throw std::runtime_error("expected any expression");
}
}  // namespace slip

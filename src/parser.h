#pragma once

#include "parcxx.h"

#include <memory>
#include <string>
#include <vector>

#include <iostream>

namespace lisp {
struct Val {
    virtual ~Val(){};
    virtual std::string Print() const = 0;
};

struct Int : public Val {
    int val;
    Int(int i) : val(i) {}
    Int(const Int& x) = default;
    virtual std::string Print() const override {
        return std::to_string(val) + ":int";
    }
};

struct Atom : public Val {
    std::string val;
    Atom(std::string s) : val(s) {}
    Atom(const Atom& x) = default;
    virtual std::string Print() const override { return val + ":atom"; }
};

struct List : public Val {
    std::vector<std::unique_ptr<Val>> vals;

    List(std::vector<std::unique_ptr<Val>> v) : vals(std::move(v)) {}
    List(List&&) = default;
    virtual std::string Print() const override {
        std::string res = "[";
        for (auto& v : vals) {
            res += v->Print() + " ";
        }
        if (res.size() > 1) {
            res.back() = ']';
        } else {
            res.push_back(']');
        }
        return res;
    }
};

template <class P>
auto Tok(P p) {
    return skip_while(parser_pred(parse_char(), isblank)) >> p;
}

auto ParseValue() {
    auto i_val =
        parse_uint() % [](auto i) { return std::unique_ptr<Val>(new Int(i)); };
    auto atom = parse_word() %
                [](auto&& s) { return std::unique_ptr<Val>(new Atom(s)); };
    return (i_val | atom);
}

auto ParseExpr() {
    Parser<std::unique_ptr<Val>> expr =
        (parse_char('(') >> list_of(Tok(recursion(expr) | ParseValue()))
                                << Tok(parse_char(')'))) %
        [](auto&& x) { return std::unique_ptr<Val>(new List(std::move(x))); };
    return expr;
}
}  // namespace lisp

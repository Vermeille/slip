#pragma once

#include <memory>
#include <string>
#include <vector>

#include "ast.h"
#include "parcxx/src/parcxx.h"

namespace slip {
namespace {
auto ParseAtom() {
    return parse_while1(
        parser_pred(parse_char(),
                    [](char c) { return !isspace(c) && c != '(' && c != ')'; }),
        std::string(),
        [](auto str, auto c) {
            str.push_back(c);
            return str;
        });
}
auto ParseStr() {
    return (parse_char('"') >>
            list_of(parser_pred(parse_char(), [](auto c) { return c != '"'; }))
                << parse_char('"')) %
           [](const auto& vec) { return std::string(&vec[0], vec.size()); };
}

template <class P>
auto Tok(P p) {
    return skip_while(parser_pred(parse_char(), isblank)) >> p;
}

auto ParseValue() {
    auto i_val =
        parse_uint() % [](auto i) { return std::unique_ptr<Val>(new Int(i)); };
    auto atom = ParseAtom() % [](auto&& s) {
        return std::unique_ptr<Val>(new Atom(std::move(s)));
    };
    auto str = ParseStr() % [](auto&& s) {
        return std::unique_ptr<Val>(new Str(std::move(s)));
    };
    return (i_val | str | atom);
}
}  // namespace

auto ParseExpr() {
    Parser<std::unique_ptr<Val>> expr =
        (parse_char('(') >> list_of(Tok(recursion(expr) | ParseValue()))
                                << Tok(parse_char(')'))) %
        [](auto&& x) { return std::unique_ptr<Val>(new List(std::move(x))); };
    return expr;
}
}  // namespace slip

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
    auto i_val = parse_uint() % [](auto i) -> Val { return Int(i); };
    auto atom =
        ParseAtom() % [](auto&& s) -> Val { return Atom(std::move(s)); };
    auto str = ParseStr() % [](auto&& s) -> Val { return Str(std::move(s)); };
    auto boolp =
        (parse_word("true") % [](const auto&) -> Val { return Bool(true); }) |
        (parse_word("false") % [](const auto&) -> Val { return Bool(false); });
    return (i_val | str | boolp | atom);
}
}  // namespace

auto ParseExpr() {
    Parser<Val> expr =
        (parse_char('(') >> list_of(Tok(recursion(expr) | ParseValue()))
                                << Tok(parse_char(')'))) %
        [](auto x) -> Val { return List(std::move(x)); };
    return expr;
}

auto Parse(const std::string& input) {
    static const auto parser = ParseExpr();
    return parser(input.begin(), input.end());
}

}  // namespace slip

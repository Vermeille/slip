#pragma once

#include "ast.h"

#include <string>

namespace slip {
struct Type {
    std::string name;
    Type() = default;
    Type(std::string n) : name(n) {}
};

class TypeChecker : public Visitor {
    Type ret_;

   public:
    void Visit(slip::Int&) override { ret_ = Type("int"); }
    void Visit(slip::Atom&) override { ret_ = Type("str"); }
    void Visit(slip::List& xs) override {
        if (xs.vals.empty()) {
            ret_ = Type("void");
            return;
        }

        std::string* fun_name = xs.GetFunName();
        if (!fun_name) {
            ret_ = Type("void");
            return;
        }

        for (size_t i = 1; i < xs.vals.size(); ++i) {
            xs.vals[i]->Accept(*this);
            *fun_name += "@" + ret_.name;
        }

        if (*fun_name == "add@int@int") {
            ret_ = Type("int");
        } else if (*fun_name == "to_string@str") {
            ret_ = Type("str");
        } else if (*fun_name == "to_string@int") {
            ret_ = Type("str");
        }
    }
};

void TypeCheck(Val& x) {
    TypeChecker tc;
    tc(x);
}
}  // namespace slip

#pragma once

#include "ast.h"
#include "context.h"
#include "function.h"
#include "mangler.h"

#include <map>
#include <string>

namespace slip {
class TypeChecker : public Visitor {
    Prototype ret_;
    Context& ctx_;

   public:
    TypeChecker(Context& ctx) : ctx_(ctx) {}

    void Visit(slip::Int&) override { ret_ = Prototype(ConstType("Int")); }
    void Visit(slip::Bool&) override { ret_ = Prototype(ConstType("Bool")); }
    void Visit(slip::Atom&) override {
        throw std::runtime_error("not implemented yet");
    }
    void Visit(slip::Str&) override { ret_ = Prototype(ConstType("String")); }
    void Visit(slip::List& xs) override {
        if (xs.empty()) {
            ret_ = Prototype(ConstType("Void"));
            return;
        }

        std::string* fun_name = xs.GetFunName();
        if (!fun_name) {
            ret_ = Prototype(ConstType("Void"));
            return;
        }

        auto found = ctx_.Find(*fun_name);
        if (!found) {
            throw std::runtime_error("Can't find function " + *fun_name);
        }

        auto ret_type = found->type();
        for (size_t i = 1; i < xs.size(); ++i) {
            xs[i]->Accept(*this);
            ret_type = ret_type.Apply(ret_);
        }
        ret_ = ret_type;
    }

    Prototype ret() const { return ret_; }
};

void TypeCheck(Val& x, Context& ctx) {
    TypeChecker tc(ctx);
    tc(x);
}

Prototype Type(Val& x, Context& ctx) {
    TypeChecker tc(ctx);
    tc(x);
    return tc.ret();
}
}  // namespace slip

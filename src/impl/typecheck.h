#pragma once

#include "ast.h"
#include "context.h"
#include "function.h"
#include "mangler.h"

#include <map>
#include <string>

namespace slip {
class TypeChecker : public Visitor {
    Type ret_;
    Context& ctx_;

   public:
    TypeChecker(Context& ctx) : ctx_(ctx) {}

    void Visit(slip::Int&) override { ret_ = Type("int"); }
    void Visit(slip::Atom&) override {
        throw std::runtime_error("not implemented yet");
    }
    void Visit(slip::Str&) override { ret_ = Type("str"); }
    void Visit(slip::List& xs) override {
        if (xs.empty()) {
            ret_ = Type("void");
            return;
        }

        std::string* fun_name = xs.GetFunName();
        if (!fun_name) {
            ret_ = Type("void");
            return;
        }

        for (size_t i = 1; i < xs.size(); ++i) {
            xs[i]->Accept(*this);
            *fun_name += "@" + ret_.name();
        }

        auto found = ctx_.Find(*fun_name);
        if (!found) {
            throw std::runtime_error("Can't resolve overload " + *fun_name);
        }
        ret_ = found->return_type();
    }
};

void TypeCheck(Val& x, Context& ctx) {
    TypeChecker tc(ctx);
    tc(x);
}
}  // namespace slip

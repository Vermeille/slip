#pragma once

#include "ast.h"
#include "context.h"
#include "function.h"
#include "mangler.h"

#include <map>
#include <string>

namespace slip {
class TypeChecker : public boost::static_visitor<Prototype> {
    Context& ctx_;

    Prototype GetFunctionType(const List& xs) {
        const std::string* fun_name = xs.GetFunName();
        if (fun_name) {
            auto found = ctx_.Find(*fun_name);
            if (!found) {
                throw std::runtime_error("Can't find function " + *fun_name);
            }
            return found->type();
        } else {
            return boost::apply_visitor(*this, xs[0]);
        }
    }

   public:
    TypeChecker(Context& ctx) : ctx_(ctx) {}

    Prototype operator()(const Int&) { return Prototype(ConstType("Int")); }
    Prototype operator()(const Bool&) { return Prototype(ConstType("Bool")); }
    Prototype operator()(const Atom&) {
        throw std::runtime_error("not implemented yet");
    }
    Prototype operator()(const Str&) { return Prototype(ConstType("String")); }
    Prototype operator()(const List& xs) {
        if (xs.empty()) {
            return Prototype(ConstType("Void"));
        }

        Prototype ret_type = GetFunctionType(xs);
        for (size_t i = 1; i < xs.size(); ++i) {
            ret_type = ret_type.Apply(boost::apply_visitor(*this, xs[i]));
        }
        return ret_type;
    }
};

void TypeCheck(Val& x, Context& ctx) {
    TypeChecker tc(ctx);
    boost::apply_visitor(tc, x);
}

Prototype TypeExpression(const Val& x, Context& ctx) {
    TypeChecker tc(ctx);
    return boost::apply_visitor(tc, x);
}
}  // namespace slip

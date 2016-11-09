#pragma once

#include <functional>
#include <string>

#include "ast.h"
#include "mangler.h"

namespace slip {
class Context;
struct Function {
    std::string mangled_name;
    Type return_type;

    virtual ~Function() = default;

    template <class R>
    R Call(const Val& x, Context& ctx) const;

   protected:
    Function(std::string fun, std::string ret)
        : mangled_name(std::move(fun)), return_type(std::move(ret)) {}
};

template <class R>
struct FunctionImpl : public Function {
   public:
    R operator()(const Val& x, Context& ctx) const { return fun_(x, ctx); }

    template <class F>
    FunctionImpl(std::string name, F&& f);

   private:
    template <class F, std::size_t... Ns>
    static R FunApply(F&& f,
                      const Val& x,
                      Context& ctx,
                      std::index_sequence<Ns...> ns);

    std::function<R(const Val&, Context&)> fun_;
};
}  // namespace slip

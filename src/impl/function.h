#pragma once

#include <functional>
#include <string>

#include "ast.h"
#include "mangler.h"

namespace slip {
class Context;
class Function {
   public:
    virtual ~Function() = default;

    template <class R>
    R Call(const Val& x, Context& ctx) const;

    const std::string& mangled_name() const { return mangled_name_; }
    const Type& return_type() const { return return_type_; }

   protected:
    enum class FunctionType { Normal, Special };

    Function(std::string fun, std::string ret, FunctionType ty)
        : mangled_name_(std::move(fun)),
          return_type_(std::move(ret)),
          type_(ty) {}

   private:
    std::string mangled_name_;
    Type return_type_;
    FunctionType type_;
};

template <class R>
class NormalFunc : public Function {
   public:
    R operator()(const Val& x, Context& ctx) const { return fun_(x, ctx); }

    template <class F>
    NormalFunc(std::string name, F&& f);

   private:
    template <class F, std::size_t... Ns>
    static R FunApply(F&& f,
                      const Val& x,
                      Context& ctx,
                      std::index_sequence<Ns...> ns);

    std::function<R(const Val&, Context&)> fun_;
};

class SpecialFunc : public Function {};
}  // namespace slip

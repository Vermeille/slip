#pragma once

#include <functional>
#include <string>

#include "ast.h"
#include "mangler.h"
#include "polymorphic.h"
#include "type.h"

namespace slip {
class Context;
class Function {
   public:
    virtual ~Function() = default;

    template <class R>
    R Call(const Val& x, Context& ctx) const;

    virtual Polymorphic operator()(const Val& x, Context& ctx) const = 0;

    const std::string& mangled_name() const { return mangled_name_; }
    const Prototype& type() const { return type_; }

   protected:
    Function(std::string fun, std::string ret)
        : mangled_name_(std::move(fun)), type_(ParseType(std::move(ret))) {}

   private:
    std::string mangled_name_;
    Prototype type_;
};

template <class R>
class NormalFunc : public Function {
   public:
    R Call(const Val& x, Context& ctx) const { return fun_(x, ctx); }

    virtual Polymorphic operator()(const Val& x, Context& ctx) const override {
        return Call(x, ctx);
    }

    template <class F>
    NormalFunc(std::string name, F&& f);

    template <class F>
    NormalFunc(std::string name, std::string type, F&& f);

   private:
    template <class F, std::size_t... Ns>
    static R FunApply(F&& f,
                      const Val& x,
                      Context& ctx,
                      std::index_sequence<Ns...> ns);

    std::function<R(const Val&, Context&)> fun_;
};

class SpecialFun : public Function {
   public:
    template <class F>
    SpecialFun(std::string name, std::string ty, F&& f)
        : Function(name, std::move(ty)), fun_(f) {}

    virtual Polymorphic operator()(const Val& x, Context& ctx) const override {
        return fun_(x, ctx);
    }

   private:
    std::function<Polymorphic(const Val&, Context&)> fun_;
};
}  // namespace slip

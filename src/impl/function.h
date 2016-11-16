#pragma once

#include <functional>
#include <string>

#include <boost/any.hpp>

#include "ast.h"
#include "mangler.h"

namespace slip {
class Context;
class Function {
   public:
    virtual ~Function() = default;

    template <class R>
    R Call(const Val& x, Context& ctx) const;

    virtual boost::any operator()(const Val& x, Context& ctx) const = 0;

    const std::string& mangled_name() const { return mangled_name_; }
    const Type& return_type() const { return return_type_; }

   protected:
    Function(std::string fun, std::string ret)
        : mangled_name_(std::move(fun)), return_type_(std::move(ret)) {}

   private:
    std::string mangled_name_;
    Type return_type_;
};

template <class R>
class NormalFunc : public Function {
   public:
    R Call(const Val& x, Context& ctx) const { return fun_(x, ctx); }

    virtual boost::any operator()(const Val& x, Context& ctx) const override {
        return Call(x, ctx);
    }

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

class SpecialFun : public Function {
   public:
    template <class F>
    SpecialFun(std::string name, F&& f) : Function(name, "BITE"), fun_(f) {}

    virtual boost::any operator()(const Val& x, Context& ctx) const override {
        return fun_(x, ctx);
    }

   private:
    std::function<boost::any(const Val&, Context&)> fun_;
};
}  // namespace slip

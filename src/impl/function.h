#pragma once

#include <functional>
#include <string>

#include "ast.h"
#include "closure.h"
#include "mangler.h"
#include "polymorphic.h"
#include "type.h"

namespace slip {
class Context;
class Function {
   public:
    virtual ~Function() = default;

    const std::string& mangled_name() const { return mangled_name_; }
    const Prototype& type() const { return type_; }

    virtual Closure GetClosure() = 0;

   protected:
    Function(std::string fun, std::string ret)
        : mangled_name_(std::move(fun)), type_(ParseType(std::move(ret))) {}

   private:
    std::string mangled_name_;
    Prototype type_;
};

template <class F>
class NormalFunc : public Function {
   public:
    NormalFunc(std::string name, F&& f);
    NormalFunc(std::string name, std::string type, F&& f);

    Closure GetClosure() override { return Closure(fun_); }

   private:
    F&& fun_;
};

#if 0
class SpecialFun : public Function {
   public:
    template <class F>
    SpecialFun(std::string name, std::string ty, F&& f)
        : Function(name, std::move(ty)), fun_(f) {}

   private:
    std::function<Polymorphic(const List&, Context&)> fun_;
};
#endif
}  // namespace slip

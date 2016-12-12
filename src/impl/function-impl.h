#pragma once

#include "function.h"

#include "context.h"
#include "eval.h"
#include "polymorphic.h"

namespace slip {

template <class F>
NormalFunc<F>::NormalFunc(std::string name, F&& f)
    : Function(name, ManglerCaller<std::decay_t<F>>::Mangle()),
      fun_(std::move(f)) {}

template <class F>
NormalFunc<F>::NormalFunc(std::string name, std::string type, F&& f)
    : Function(name, type), fun_(std::move(f)) {}

}  // namespace slip

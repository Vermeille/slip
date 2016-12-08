#pragma once

#include "function.h"

#include "context.h"
#include "eval.h"
#include "polymorphic.h"

namespace slip {

namespace {
template <class T>
T CallImpl(const Function& fun, const Val& x, Context& ctx) {
    if (auto i = dynamic_cast<const NormalFunc<T>*>(&fun)) {
        return i->Call(x, ctx);
    }
    return (fun(x, ctx)).as<T>();
}

template <>
Polymorphic CallImpl<Polymorphic>(const Function& fun,
                                  const Val& x,
                                  Context& ctx) {
    return fun(x, ctx);
}
}  // anon namespace

template <class R>
template <class F>
NormalFunc<R>::NormalFunc(std::string name, F&& f)
    : Function(name, ManglerCaller<std::decay_t<F>>::Mangle()),
      fun_([ f = std::move(f), name = name ](const Val& x, Context & ctx)->R {
          return FunApply(f,
                          name,
                          x,
                          ctx,
                          std::make_index_sequence<
                              ManglerCaller<std::decay_t<F>>::arity>());
      }) {}

template <class R>
template <class F>
NormalFunc<R>::NormalFunc(std::string name, std::string type, F&& f)
    : Function(name, type),
      fun_([ f = std::move(f), name = name ](const Val& x, Context & ctx)->R {
          return FunApply(f,
                          name,
                          x,
                          ctx,
                          std::make_index_sequence<
                              ManglerCaller<std::decay_t<F>>::arity>());
      }) {}

template <class R>
R Function::Call(const Val& x, Context& ctx) const {
    return CallImpl<R>(*this, x, ctx);
}

template <class F>
using _FunArgs = typename ManglerCaller<std::decay_t<F>>::args_type;

template <int N, class F>
using _NthArg =
    std::decay_t<decltype(std::get<N>(std::declval<_FunArgs<F>>()))>;

template <class R>
template <class F, std::size_t... Ns>
R NormalFunc<R>::FunApply(F&& f,
                          const std::string& name,
                          const Val& x,
                          Context& ctx,
                          std::index_sequence<Ns...>) {
    auto* args = dynamic_cast<const List*>(&x);
    if (!args || args->size() != sizeof...(Ns) + 1) {
        throw std::runtime_error("Can't invoke " + name + " expected " +
                                 std::to_string(sizeof...(Ns)) +
                                 " arguments, got " +
                                 std::to_string(args->size() - 1));
    }
    return f(Eval<_NthArg<Ns, F>>(*(*args)[Ns + 1], ctx)...);
}

}  // namespace slip

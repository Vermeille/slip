#pragma once

#include "function.h"

#include "context.h"
#include "eval.h"

namespace slip {

namespace {
template <class T>
T CallImpl(const Function& fun, const Val& x, Context& ctx) {
    if (auto i = dynamic_cast<const NormalFunc<T>*>(&fun)) {
        return i->Call(x, ctx);
    }
    return boost::any_cast<T>(fun(x, ctx));
}

template <>
boost::any CallImpl<boost::any>(const Function& fun,
                                const Val& x,
                                Context& ctx) {
    return fun(x, ctx);
}
}  // anon namespace

template <class R>
template <class F>
NormalFunc<R>::NormalFunc(std::string name, F&& f)
    : Function(name, ManglerCaller<std::decay_t<F>>::Mangle()),
      fun_([f = std::move(f)](const Val& x, Context & ctx)->R {
          return FunApply(f,
                          x,
                          ctx,
                          std::make_index_sequence<
                              ManglerCaller<std::decay_t<F>>::arity>());
      }) {}

template <class R>
R Function::Call(const Val& x, Context& ctx) const {
    return CallImpl<R>(*this, x, ctx);
}

template <class R>
template <class F, std::size_t... Ns>
R NormalFunc<R>::FunApply(F&& f,
                          const Val& x,
                          Context& ctx,
                          std::index_sequence<Ns...>) {
    // HAIL SATAN
    return f(Eval<std::decay_t<decltype(std::get<Ns>(
                 std::declval<
                     typename ManglerCaller<std::decay_t<F>>::args_type>()))>>(
        *dynamic_cast<const List&>(x)[Ns + 1], ctx)...);
}

}  // namespace slip

#pragma once

#include "function.h"

#include "context.h"
#include "eval.h"

namespace slip {
template <class R>
template <class F>
FunctionImpl<R>::FunctionImpl(std::string name, F&& f)
    : Function(name + ManglerCaller<std::decay_t<F>>::Mangle(),
               ManglerCaller<std::decay_t<F>>::Result()),
      fun_([f = std::move(f)](const Val& x, Context & ctx)->R {
          return FunApply(f,
                          x,
                          ctx,
                          std::make_index_sequence<
                              ManglerCaller<std::decay_t<F>>::arity>());
      }) {}

template <class R>
R Function::Call(const Val& x, Context& ctx) const {
    return static_cast<const FunctionImpl<R>&>(*this)(x, ctx);
}

template <class R>
template <class F, std::size_t... Ns>
R FunctionImpl<R>::FunApply(F&& f,
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

#pragma once

#include <string>
#include <type_traits>

#include "type.h"

namespace slip {
template <class T>
struct GetTypeId : public GetTypeId<std::decay_t<T>> {};

template <>
struct GetTypeId<int> {
    static std::string type() { return "Int"; }
};

template <>
struct GetTypeId<bool> {
    static std::string type() { return "Bool"; }
};

template <>
struct GetTypeId<std::string> {
    static std::string type() { return "String"; }
};

template <class... Args>
struct Mangler;

template <class T>
struct Mangler<T> {
    static const std::string Mangle() { return GetTypeId<T>::type(); }
};

template <class A, class... Args>
struct Mangler<A, Args...> {
    static const std::string Mangle() {
        return GetTypeId<A>::type() + " -> " + Mangler<Args...>::Mangle();
    }
};

template <class F>
struct ManglerCaller : public ManglerCaller<decltype(&F::operator())> {
    using Impl = ManglerCaller<decltype(&F::operator())>;
    static const std::string Mangle() { return Impl::Mangle(); }
    static const std::string Result() { return Impl::Result(); }
    using result_type = typename Impl::result_type;
    using args_type = typename Impl::args_type;
    static constexpr int arity = Impl::arity;
};

template <class R, class... Args>
struct ManglerCaller<R (*)(Args...)> {
    static const std::string Mangle() {
        return Mangler<Args...>::Mangle() + " -> " + Result();
    }
    static const std::string Result() { return GetTypeId<R>::type(); }
    typedef R result_type;
    typedef std::tuple<std::decay_t<Args>...> args_type;
    static constexpr int arity = sizeof...(Args);
};

template <class R, class C, class... Args>
struct ManglerCaller<R (C::*)(Args...) const> {
    static const std::string Mangle() {
        return Mangler<Args...>::Mangle() + " -> " + Result();
    }
    static const std::string Result() { return GetTypeId<R>::type(); }
    typedef R result_type;
    typedef std::tuple<std::decay_t<Args>...> args_type;
    static constexpr int arity = sizeof...(Args);
};
}  // namespace slip

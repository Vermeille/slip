#pragma once

#include <string>
#include <type_traits>

namespace slip {
struct Type {
    std::string name;
    Type() = default;
    Type(std::string n) : name(n) {}
};

template <class T>
struct GetTypeId : public GetTypeId<std::decay_t<T>> {};

template <>
struct GetTypeId<int> {
    static const Type type() { return Type("int"); }
};

template <>
struct GetTypeId<bool> {
    static const Type type() { return Type("bool"); }
};

template <>
struct GetTypeId<std::string> {
    static const Type type() { return Type("str"); }
};

template <class... Args>
struct Mangler;

template <>
struct Mangler<> {
    static const std::string Mangle() { return ""; }
};

template <class A, class... Args>
struct Mangler<A, Args...> {
    static const std::string Mangle() {
        return "@" + GetTypeId<A>::type().name + Mangler<Args...>::Mangle();
    }
};

template <class F>
struct ManglerCaller : public ManglerCaller<decltype(&F::operator())> {};

template <class R, class... Args>
struct ManglerCaller<R (*)(Args...)> {
    static const std::string Mangle() { return Mangler<Args...>::Mangle(); }
    static const std::string Result() { return GetTypeId<R>::type(); }
    typedef R result_type;
    typedef std::tuple<Args...> args_type;
    static constexpr int arity = sizeof...(Args);
};

template <class R, class C, class... Args>
struct ManglerCaller<R (C::*)(Args...) const> {
    static const std::string Mangle() { return Mangler<Args...>::Mangle(); }
    static const std::string Result() { return GetTypeId<R>::type().name; }
    typedef R result_type;
    typedef std::tuple<Args...> args_type;
    static constexpr int arity = sizeof...(Args);
};
}  // namespace slip

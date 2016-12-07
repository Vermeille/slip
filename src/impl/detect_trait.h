#pragma once

#include <type_traits>

template <class...>
using void_t = void;

template <class Lol, template <class...> class Op, class... Args>
struct Detect_ {
    static constexpr bool value = false;
};

template <template <class...> class Op, class... Args>
struct Detect_<void_t<Op<Args...>>, Op, Args...> {
    static constexpr bool value = true;
};

template <template <class...> class Op, class... Args>
using Detect = Detect_<void, Op, Args...>;

template <class T, class U>
using has_addition = decltype(std::declval<T>() + std::declval<U>());

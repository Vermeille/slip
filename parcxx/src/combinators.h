#pragma once

#include "optional.h"

#include <functional>
#include <string>
#include <tuple>
#include <vector>

typedef std::string::const_iterator str_iterator;

template <class F>
class ParserImpl {
    const F fun_;

   public:
    template <class T>
    friend class ParserImpl;

    ParserImpl(const F& f) : fun_(f) {}
    ParserImpl(F&& f) : fun_(std::move(f)) {}

    template <class F2>
    ParserImpl(ParserImpl<F2>&& f) : fun_(std::move(f.fun_)) {}

    template <class F2>
    ParserImpl(const ParserImpl<F2>& f) : fun_(f.fun_) {}

    decltype(auto) operator()(str_iterator begin, str_iterator end) const {
        return fun_(begin, end);
    }
};

template <class T>
using ParserRet = optional<std::pair<T, std::string::const_iterator>>;

template <class T>
using Parser =
    ParserImpl<std::function<ParserRet<T>(str_iterator, str_iterator)>>;

template <class F>
decltype(auto) make_parser(F&& f) {
    return ParserImpl<F>(std::forward<F>(f));
}

template <class T>
using parser_ret_type =
    decltype(std::declval<T>()(str_iterator(), str_iterator())->first);

class Empty {};

auto parse_char() {
    return make_parser([](str_iterator begin, str_iterator end) {
        if (begin != end) {
            return make_optional(std::make_pair(*begin, begin + 1));
        }
        return ParserRet<char>();
    });
}

template <class Par, class Pred>
auto parser_pred(Par par, Pred p) {
    return make_parser([par, p](str_iterator begin,
                                str_iterator end) -> decltype(par(begin, end)) {
        auto res = par(begin, end);
        if (!res) {
            return decltype(res)();
        }
        if (p(res->first)) {
            return res;
        }
        return decltype(res)();
    });
}

template <class P, class T, class R = Empty>
auto parse_seq(P p, T str, R x = R()) {
    return make_parser([=](str_iterator begin, str_iterator end) {
        ParserRet<decltype(p(end, end)->first)> ret;
        auto iter = begin;
        T str2 = str;
        for (; *str2; ++str2) {
            ret = parser_pred(p, [=](auto c) { return c == *str2; })(iter, end);
            if (!ret) {
                return ParserRet<R>();
            }
            iter = ret->second;
        }
        return make_optional(std::make_pair(x, ret->second));
    });
}

template <class P1, class P2>
auto pthen(P1 p1, P2 p2) {
    return make_parser([=](str_iterator begin, str_iterator end) {
        auto res = p1(begin, end);
        if (res) {
            return p2(std::move(res->first))(res->second, end);
        }
        return decltype(p2(std::move(res->first))(res->second, end))();
    });
}

template <class P1, class P2>
auto operator>>=(ParserImpl<P1> p1, P2 p2) {
    return pthen(p1, p2);
}

template <class P1>
auto poptional(P1 p1) {
    return make_parser([=](str_iterator begin, str_iterator end) {
        auto res = p1(begin, end);
        if (res) {
            return make_optional(
                std::make_pair(make_optional(res->first), res->second));
        }
        return make_optional(
            std::make_pair(optional<decltype(res->first)>(), begin));
    });
}

template <class P1>
auto operator!(ParserImpl<P1> p1) {
    return poptional(p1);
}

template <class T, class F>
auto apply(T&& x, F&& f) {
    return make_parser(
        [ f = std::move(f), x ](str_iterator begin, str_iterator end)
            ->decltype(auto) {
                auto res = x(begin, end);
                if (!res) {
                    return ParserRet<typename std::decay<decltype(
                        f(std::move(res->first)))>::type>();
                }
                return make_optional(
                    std::make_pair(f(std::move(res->first)), res->second));
            });
}

template <class T, class F>
auto operator%(ParserImpl<T> x, F&& f) {
    return apply(x, f);
}

namespace {
template <class T>
struct to_tuple;

template <class... Ts>
struct to_tuple<std::tuple<Ts...>> {
    template <class U>
    static decltype(auto) do_it(U&& x) {
        return std::forward<U>(x);
    }
};

template <class T, class U>
struct to_tuple<std::pair<T, U>> {
    template <class V>
    static decltype(auto) do_it(V&& x) {
        return std::forward<V>(x);
    }
};

template <class T>
struct to_tuple {
    template <class U>
    static decltype(auto) do_it(U&& x) {
        return std::make_tuple(std::forward<U>(x));
    }
};

template <class T, class U>
decltype(auto) as_pair(std::tuple<T, U>&& tup) {
    return std::make_pair(std::move(std::get<0>(tup)),
                          std::move(std::get<1>(tup)));
}

template <class... Ts>
decltype(auto) as_pair(std::tuple<Ts...>&& tup) {
    return std::move(tup);
}
}

template <class T, class U>
auto chain(T a, U b) {
    return make_parser([=](str_iterator begin,
                           str_iterator end) -> decltype(auto) {
        using result_type =
            ParserRet<typename std::decay<decltype(as_pair(std::tuple_cat(
                to_tuple<typename std::decay<decltype(a(begin, end)->first)>::
                             type>::do_it(std::move(a(begin, end)->first)),
                to_tuple<
                    typename std::decay<decltype(b(begin, end)->first)>::type>::
                    do_it(std::move(b(begin, end)->first)))))>::type>;

        auto r = a(begin, end);
        if (!r) {
            return result_type();
        }
        auto r2 = b(r->second, end);
        if (!r2) {
            return result_type();
        }
        return make_optional(std::make_pair(
            as_pair(std::tuple_cat(
                to_tuple<typename std::decay<decltype(r->first)>::type>::do_it(
                    std::move(r->first)),
                to_tuple<typename std::decay<decltype(r2->first)>::type>::do_it(
                    std::move(r2->first)))),
            r2->second));
        ;
    });
}

template <class P1, class P2>
auto operator&(ParserImpl<P1> p1, ParserImpl<P2> p2) {
    return chain(p1, p2);
}

template <class T, class U>
auto parse_pair(T a, U b) {
    return make_parser([=](str_iterator begin, str_iterator end) {
        typedef ParserRet<std::pair<decltype(a(begin, end)->first),
                                    decltype(b(begin, end)->first)>>
            result_type;

        auto r = a(begin, end);
        if (!r) {
            return result_type();
        }
        auto r2 = b(r->second, end);
        if (!r2) {
            return result_type();
        }
        return make_optional(std::make_pair(
            std::make_pair(std::move(r->first), std::move(r2->first)),
            r2->second));
    });
}

template <class P1, class P2>
auto operator+(ParserImpl<P1> p1, ParserImpl<P2> p2) {
    return parse_pair(p1, p2);
}

template <class T, class U>
auto skipL(T a, U b) {
    return (a + b) %
               [](auto&& x) -> decltype(auto) { return std::move(x.second); };
}

template <class T, class U>
auto operator>>(ParserImpl<T> a, ParserImpl<U> b) {
    return skipL(a, b);
}

template <class T, class U>
auto skipR(T a, U b) {
    return (a + b) %
               [](auto&& r) -> decltype(auto) { return std::move(r.first); };
}

template <class T, class U>
auto operator<<(ParserImpl<T> a, ParserImpl<U> b) {
    return skipR(a, b);
}

template <class P, class U, class F>
auto parse_while(P p, U u, F f) {
    return make_parser([=](str_iterator begin, str_iterator end) {
        ParserRet<U> ret(std::make_pair(u, begin));
        while (1) {
            auto res = p(ret->second, end);
            if (!res) {
                return ret;
            }
            ret = ParserRet<U>(
                std::make_pair(f(ret->first, res->first), res->second));
        }
    });
}

template <class P>
auto skip_while(P p) {
    return parse_while(p, Empty(), [](auto, auto) { return Empty(); });
}

template <class P, class U, class F>
auto parse_while1(P p, U u, F f) {
    return p >>= [=](auto&& res) { return parse_while(p, f(u, res), f); };
}

template <class P>
auto skip_while1(P p) {
    return parse_while1(p, Empty(), [](auto, auto) { return Empty(); });
}

template <class... Ts>
auto parse_try(Ts... p1);

template <class T>
auto parse_try(T p) {
    return p;
}

template <class T, class... Ts>
auto parse_try(T p, Ts... ps) {
    return make_parser([=](str_iterator begin, str_iterator end) {
        auto res = p(begin, end);
        if (res) {
            return res;
        }
        return parse_try(ps...)(begin, end);
    });
}

template <class P1, class P2>
auto operator|(ParserImpl<P1> p1, ParserImpl<P2> p2) {
    return parse_try(p1, p2);
}

template <class T>
auto parse_nothing(T x) {
    return make_parser([=](str_iterator begin, str_iterator) {
        return make_optional(std::make_pair(x, begin));
    });
}

template <class P>
auto list_of(P&& p) {
    return make_parser([=](str_iterator b, str_iterator e) {
        std::vector<decltype(p(str_iterator(), str_iterator())->first)> vec;

        auto res = parse_while1(p, &vec, [](auto&& vecptr, auto&& x) {
            vecptr->push_back(std::move(x));
            return vecptr;
        })(b, e);

        if (!res) {
            return ParserRet<decltype(vec)>();
        }
        return make_optional(std::make_pair(std::move(vec), res->second));
    });
};

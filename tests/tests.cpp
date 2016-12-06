#include "slip.h"

#include <iostream>

#include "impl/type.h"

auto ParseSlip(const std::string& input) {
    static const auto parser = slip::ParseExpr();
    return parser(input.begin(), input.end());
}

template <class T>
void expect_eq(std::string in, std::string parse, T x, slip::Context& ctx) {
    using namespace slip;
    std::cout << in << "\n";
    auto res = ParseSlip(in);
    TypeCheck(*res->first, ctx);
    auto printed = slip::Print(*res->first);
    std::cout << "=> " << printed << "\n";
    assert(printed == parse);
    auto eval = Eval<std::decay_t<T>>(*res->first, ctx);
    assert(eval == x);
    std::cout << "=> " << eval << "\n";
}

int main() {
    using namespace slip;

    Context ctx;
    ctx.ImportBase();
    expect_eq("(return 1)", "[return:atom 1:int]", 1, ctx);
    expect_eq("(return \"lol\")",
              "[return:atom \"lol\":str]",
              std::string("lol"),
              ctx);
    expect_eq("(return (+ 1 2))", "[return:atom [+:atom 1:int 2:int]]", 3, ctx);
    expect_eq("(+ 1 2)", "[+:atom 1:int 2:int]", 3, ctx);
    expect_eq("(+ 1 (+ 1 1))", "[+:atom 1:int [+:atom 1:int 1:int]]", 3, ctx);

    expect_eq("(+s \"Werez my \" \"SLIP?\")",
              "[+s:atom \"Werez my \":str \"SLIP?\":str]",
              std::string("Werez my SLIP?"),
              ctx);

    expect_eq("(and (< 2 3) (== 1 1))",
              "[and:atom [<:atom 2:int 3:int] [==:atom 1:int 1:int]]",
              true,
              ctx);

    Prototype f1(Arrow(ConstType("Int"), ConstType("Int")));
    assert(f1.Show() == "Int -> Int");

    Prototype fif(Arrow(ConstType("Bool"),
                        Arrow(TypeVar(1), Arrow(TypeVar(1), TypeVar(1)))));
    assert(fif.Show() == "forall t1. Bool -> t1 -> t1 -> t1");

    try {
        fif.Apply(Prototype(ConstType("Int")));
        assert(false);
    } catch (...) {
    }

    auto res = fif.Apply(Prototype(ConstType("Bool")));
    res = res.Apply(Prototype(Arrow(ConstType("Int"), ConstType("Bool"))));
    assert(res.Show() == "(Int -> Bool) -> Int -> Bool");

    Prototype constf(Arrow(TypeVar(1), Arrow(TypeVar(1), TypeVar(2))));
    Prototype a_fun(Arrow(TypeVar(3), TypeVar(4)));
    auto applied = constf.Apply(a_fun);
    assert(applied.Show() == "forall t2 t3 t4. (t3 -> t4) -> t2");

    Prototype b_fun = a_fun;
    Namer namer;
    b_fun.Instantiate(namer);
    assert(b_fun.Show() == "forall t1 t2. t1 -> t2");

    expect_eq("(if (== 1 1) 42 666)",
              "[if:atom [==:atom 1:int 1:int] 42:int 666:int]",
              42,
              ctx);

    expect_eq("(if (== 0 1) 42 666)",
              "[if:atom [==:atom 0:int 1:int] 42:int 666:int]",
              666,
              ctx);
    return 0;
}

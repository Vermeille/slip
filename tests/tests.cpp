#include "slip.h"

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
    expect_eq("(return 1)", "[return@int:atom 1:int]", 1, ctx);
    expect_eq("(return \"lol\")",
              "[return@str:atom \"lol\":str]",
              std::string("lol"),
              ctx);
    expect_eq("(return (+ 1 2))",
              "[return@int:atom [+@int@int:atom 1:int 2:int]]",
              3,
              ctx);
    expect_eq("(+ 1 2)", "[+@int@int:atom 1:int 2:int]", 3, ctx);
    expect_eq("(+ 1 (+ 1 1))",
              "[+@int@int:atom 1:int [+@int@int:atom 1:int 1:int]]",
              3,
              ctx);

    expect_eq("(+ \"Werez my \" \"SLIP?\")",
              "[+@str@str:atom \"Werez my \":str \"SLIP?\":str]",
              std::string("Werez my SLIP?"),
              ctx);

    expect_eq("(and (< 2 3) (== 1 1))",
              "[and@bool@bool:atom [<@int@int:atom 2:int 3:int] "
              "[==@int@int:atom 1:int 1:int]]",
              true,
              ctx);

    using namespace slip::experimental;
    using slip::experimental::Type;

    Prototype f1(Arrow(ConstType("Int"), ConstType("Int")));
    Prototype f2 = f1;
    std::cout << f1.Show() << "\n";
    std::cout << f2.Show() << "\n";

    Prototype fif(Arrow(ConstType("Bool"),
                        Arrow(TypeVar(1), Arrow(TypeVar(1), TypeVar(1)))));
    std::cout << fif.Show() << "\n";

    try {
        fif.Apply(AtomType("Int"));
        assert(false);
    } catch (...) {
    }
    assert(fif.Apply(AtomType("Bool")).IsValid());

    auto app = fif.Apply(AtomType("Bool"));
    std::cout << app.Show() << "\n";

    app = app.Apply(FunctionType(AtomType("a"), AtomType("b")));
    std::cout << app.Show() << "\n";

    return 0;
}

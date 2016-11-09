#include "parser.h"
#include "print.h"
#include "typecheck.h"

#include <cassert>

template <class T>
T Read(const std::unique_ptr<slip::Val>& v);

template <>
std::string Read<std::string>(const std::unique_ptr<slip::Val>& v) {
    if (slip::Atom* i = dynamic_cast<slip::Atom*>(v.get())) {
        return i->val();
    } else if (slip::Str* i = dynamic_cast<slip::Str*>(v.get())) {
        return i->val();
    } else if (slip::List* i = dynamic_cast<slip::List*>(v.get())) {
        std::string fun = Read<std::string>((*i)[0]);
        if (fun == "to_string@int") {
            return Read<std::string>((*i)[1]);
        } else if (fun == "to_string@str") {
            return Read<std::string>((*i)[1]);
        } else if (fun == "return") {
            return Read<std::string>((*i)[1]);
        }
    }
    throw std::runtime_error("expected a str expression");
}

template <>
int Read<int>(const std::unique_ptr<slip::Val>& v) {
    if (slip::Int* i = dynamic_cast<slip::Int*>(v.get())) {
        return i->val();
    } else if (slip::List* i = dynamic_cast<slip::List*>(v.get())) {
        std::string fun = Read<std::string>((*i)[0]);
        if (fun == "return") {
            return Read<int>((*i)[1]);
        } else if (fun == "add@int@int") {
            return Read<int>((*i)[1]) + Read<int>((*i)[2]);
        }
    }
    throw std::runtime_error("expected an int expression");
}

auto ParseSlip(const std::string& input) {
    static const auto parser = slip::ParseExpr();
    return parser(input.begin(), input.end());
}

template <class T>
void expect_eq(std::string in, std::string parse, T x) {
    using namespace slip;
    std::cout << in << "\n";
    auto res = ParseSlip(in);
    auto printed = slip::Print(*res->first);
    std::cout << "=> " << printed << "\n";
    assert(printed == parse);
    auto eval = Read<T>(res->first);
    assert(eval == x);
    std::cout << "=> " << eval << "\n";
}

#if 0
void test_mangling(std::string in, std::string mangled) {
    using namespace slip;
    std::cout << in << "\n";
    auto res = ParseSlip(in);
    TypeCheck(*res->first);
    auto printed = slip::Print(*res->first);
    std::cout << "=> " << printed << "\n";
    assert(printed == mangled);
}
#endif

int main() {
    expect_eq("(return 1)", "[return:atom 1:int]", 1);
    expect_eq(
        "(return \"lol\")", "[return:atom \"lol\":str]", std::string("lol"));
    expect_eq("(return (add@int@int 1 2))",
              "[return:atom [add@int@int:atom 1:int 2:int]]",
              3);
    expect_eq("(add@int@int 1 2)", "[add@int@int:atom 1:int 2:int]", 3);
    expect_eq("(add@int@int 1 (add@int@int 1 1))",
              "[add@int@int:atom 1:int [add@int@int:atom 1:int 1:int]]",
              3);
#if 0
    test_mangling("(add 1 1)", "[add@int@int:atom 1:int 1:int]");
    test_mangling("(add a 1)", "[add@str@int:atom a:atom 1:int]");
#endif
    slip::CompilationContext ctx;
    for (auto& f : ctx.functions_) {
        std::cout << f.first << "\n";
    }
    ctx.DeclareFun("+", [](int, int) -> int { return -1; });
    std::string input("(+ 1 1)");
    auto parsed = ParseSlip(input);
    auto printed = slip::Print(*parsed->first);
    std::cout << "=> " << printed << "\n";
    assert(parsed);
    TypeCheck(*parsed->first, ctx);
    printed = slip::Print(*parsed->first);
    std::cout << "=> " << printed << "\n";

    return 0;
}

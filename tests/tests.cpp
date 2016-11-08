#include "parser.h"
#include "print.h"
#include "typecheck.h"

#include <cassert>

template <class T>
T Read(const std::unique_ptr<slip::Val>& v);

template <>
std::string Read<std::string>(const std::unique_ptr<slip::Val>& v) {
    if (slip::Atom* i = dynamic_cast<slip::Atom*>(v.get())) {
        return i->val;
    } else if (slip::List* i = dynamic_cast<slip::List*>(v.get())) {
        std::string fun = Read<std::string>(i->vals[0]);
        if (fun == "to_string@int") {
            return Read<std::string>(i->vals[1]);
        } else if (fun == "to_string@str") {
            return Read<std::string>(i->vals[1]);
        }
    }
    throw std::runtime_error("expected an atom expression");
}

template <>
int Read<int>(const std::unique_ptr<slip::Val>& v) {
    if (slip::Int* i = dynamic_cast<slip::Int*>(v.get())) {
        return i->val;
    } else if (slip::List* i = dynamic_cast<slip::List*>(v.get())) {
        std::string fun = Read<std::string>(i->vals[0]);
        if (fun == "return") {
            return Read<int>(i->vals[1]);
        } else if (fun == "add@int@int") {
            return Read<int>(i->vals[1]) + Read<int>(i->vals[2]);
        }
    }
    throw std::runtime_error("expected an int expression");
}

template <class T>
void expect_eq(std::string in, std::string parse, T x) {
    using namespace slip;
    std::cout << in << "\n";
    auto parser = slip::ParseExpr();
    auto res = parser(in.begin(), in.end());
    auto printed = slip::Print(*res->first);
    std::cout << "=> " << printed << "\n";
    assert(printed == parse);
    auto eval = Read<T>(res->first);
    assert(eval == x);
    std::cout << "=> " << eval << "\n";
}

void test_mangling(std::string in, std::string mangled) {
    using namespace slip;
    std::cout << in << "\n";
    auto parser = slip::ParseExpr();
    auto res = parser(in.begin(), in.end());
    TypeCheck(*res->first);
    auto printed = slip::Print(*res->first);
    std::cout << "=> " << printed << "\n";
    assert(printed == mangled);
}

int main() {
    expect_eq("(return 1)", "[return:atom 1:int]", 1);
    expect_eq("(return (add@int@int 1 2))",
              "[return:atom [add@int@int:atom 1:int 2:int]]",
              3);
    expect_eq("(add@int@int 1 2)", "[add@int@int:atom 1:int 2:int]", 3);
    expect_eq("(add@int@int 1 (add@int@int 1 1))",
              "[add@int@int:atom 1:int [add@int@int:atom 1:int 1:int]]",
              3);
    test_mangling("(add 1 1)", "[add@int@int:atom 1:int 1:int]");
    test_mangling("(add a 1)", "[add@str@int:atom a:atom 1:int]");
    return 0;
}

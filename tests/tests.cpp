#include "parser.h"

#include <cassert>

int main() {
    using namespace lisp;
    auto parser = lisp::ParseExpr();
    std::string in("( 1 a (b))");
    auto res = parser(in.begin(), in.end());
    assert(res->first->Print() == "[1:int a:atom [b:atom]]");
}

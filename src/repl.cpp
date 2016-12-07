#include "slip.h"

#include <iostream>

int main() {
    slip::Context ctx;
    ctx.ImportBase();

    std::string line;
    std::cout << "> ";
    while (std::getline(std::cin, line)) {
        auto res = slip::Parse(line);
        if (!res) {
            std::cerr << "Parse error\n";
        } else {
            TypeCheck(*res->first, ctx);
            auto eval = slip::Eval<slip::Polymorphic>(*res->first, ctx);
            std::cout << eval.Show() << "\n";
        }
        std::cout << "> ";
    }
    return 0;
}

#include "slip.h"

#include <iostream>

template <>
struct slip::GetTypeId<slip::Prototype> {
    static std::string type() { return "Prototype"; }
};

int main() {
    slip::Context ctx;
    ctx.ImportBase();

    ctx.DeclareSpecial("!type",
                       "a -> Polymorphic",
                       [](const slip::Val& x, slip::Context& ctx) {
                           const slip::List* l;
                           if (!(l = dynamic_cast<const slip::List*>(&x))) {
                               throw std::runtime_error("invalid arg to if");
                           }
                           return slip::Type(*(*l)[1], ctx);
                       });

    ctx.DeclareFun("parsetype",
                   [](const std::string& str) { return slip::ParseType(str); });

    ctx.DeclareFun("applytype",
                   [](const slip::Prototype& type, const slip::Prototype& arg) {
                       return type.Apply(arg);
                   });
    ctx.DeclareFun("substype",
                   [](const slip::Prototype& type,
                      const std::string& namevar,
                      const slip::Prototype& arg) {
                       return type.Substitue(slip::LowerCaseIdToNbr(namevar),
                                             arg);
                   });

    std::string line;
    std::cout << "> ";
    while (std::getline(std::cin, line)) {
        auto res = slip::Parse(line);
        try {
            if (!res) {
                std::cerr << "Parse error\n";
            } else {
                TypeCheck(*res->first, ctx);
                auto eval = slip::Eval<slip::Polymorphic>(*res->first, ctx);
                std::cout << eval.Show() << "\n";
            }
        } catch (std::exception& e) {
            std::cerr << "ERROR: " << e.what() << "\n";
        }
        std::cout << "> ";
    }
    return 0;
}

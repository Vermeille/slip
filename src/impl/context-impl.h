#pragma once

#include "context.h"
#include "eval.h"
#include "polymorphic.h"

namespace slip {
template <class F>
void Context::DeclareFun(std::string name, F&& f) {
    auto ptr = std::unique_ptr<Function>(
        new NormalFunc<F>(std::move(name), std::move(f)));
    functions_[ptr->mangled_name()] = std::move(ptr);
}

template <class F>
void Context::DeclareFun(std::string name, std::string type, F&& f) {
    auto ptr = std::unique_ptr<Function>(
        new NormalFunc<F>(std::move(name), std::move(type), std::move(f)));
    functions_[ptr->mangled_name()] = std::move(ptr);
}

template <class F>
void Context::DeclareSpecial(std::string name, std::string ty, F&& f) {
#if 0
    auto ptr = std::unique_ptr<Function>(
        new SpecialFun(std::move(name), std::move(ty), std::move(f)));
    functions_[ptr->mangled_name()] = std::move(ptr);
#endif
}

Function* Context::Find(const std::string& name) const {
    auto found = functions_.find(name);
    if (found != functions_.end()) {
        return found->second.get();
    }
    return nullptr;
}

void Context::Dump() const {
    for (auto& x : functions_) {
        std::cout << x.second->mangled_name()
                  << " :: " << x.second->type().Show() << "\n";
    }
}

void Context::ImportBase() {
    DeclareFun("+", [](int a, int b) -> int { return a + b; });
    DeclareFun("+s",
               [](const std::string& a, const std::string& b) -> std::string {
                   return a + b;
               });
    DeclareFun("*", [](int a, int b) -> int { return a * b; });
    DeclareFun("-", [](int a, int b) -> int { return a - b; });
    DeclareFun("%", [](int a, int b) -> int { return a % b; });
    DeclareFun("/", [](int a, int b) -> int { return a / b; });

    DeclareFun("==", [](int a, int b) -> bool { return a == b; });
    DeclareFun("<=", [](int a, int b) -> bool { return a <= b; });
    DeclareFun(">=", [](int a, int b) -> bool { return a >= b; });
    DeclareFun("<", [](int a, int b) -> bool { return a < b; });
    DeclareFun(">", [](int a, int b) -> bool { return a > b; });

    DeclareFun("not", [](bool a) -> bool { return !a; });
    DeclareFun("and", [](bool a, bool b) -> bool { return a && b; });
    DeclareFun("or", [](bool a, bool b) -> bool { return a || b; });

#if 0
    DeclareSpecial(
        "if", "Bool -> a -> a -> a", [](const List& l, Context& ctx) {
            if (l.size() != 4) {
                throw std::runtime_error("'if' expects 3 arguments");
            }
            return Eval<bool>(l[1], ctx) ? Eval<Polymorphic>(l[2], ctx)
                                         : Eval<Polymorphic>(l[3], ctx);
        });
#endif

    DeclareFun("return", "a -> a", [](const Polymorphic& x) { return x; });
    DeclareFun("const",
               "a -> b -> a",
               [](const Polymorphic& a, const Polymorphic&) { return a; });
}
}  // namespace slip

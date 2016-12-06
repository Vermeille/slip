#pragma once

#include "context.h"

#include "eval.h"

namespace slip {
template <class F>
void Context::DeclareFun(std::string name, F&& f) {
    auto ptr = std::unique_ptr<Function>(
        new NormalFunc<typename ManglerCaller<F>::result_type>(std::move(name),
                                                               std::move(f)));
    functions_[ptr->mangled_name()] = std::move(ptr);
}

template <class F>
void Context::DeclareSpecial(std::string name, std::string ty, F&& f) {
    auto ptr = std::unique_ptr<Function>(
        new SpecialFun(std::move(name), std::move(ty), std::move(f)));
    functions_[ptr->mangled_name()] = std::move(ptr);
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
    DeclareFun("+s", [](std::string a, std::string b) -> std::string {
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

    DeclareSpecial("if", "Bool -> a -> a -> a", [](const Val& x, Context& ctx) {
        const List* l;
        if (!(l = dynamic_cast<const List*>(&x))) {
            throw std::runtime_error("invalid arg to if");
        }
        return Eval<bool>(*(*l)[1], ctx) ? Eval<boost::any>(*(*l)[2], ctx)
                                         : Eval<boost::any>(*(*l)[3], ctx);
    });

    DeclareSpecial("return", "a -> a", [](const Val& x, Context& ctx) {
        const List* l;
        if (!(l = dynamic_cast<const List*>(&x))) {
            throw std::runtime_error("invalid arg to ");
        }
        return Eval<boost::any>(*(*l)[1], ctx);
    });
}
}  // namespace slip

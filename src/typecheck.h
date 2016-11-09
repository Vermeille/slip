#pragma once

#include "ast.h"

#include <map>
#include <string>

namespace slip {

struct Type {
    std::string name;
    Type() = default;
    Type(std::string n) : name(n) {}
};

template <class T>
struct GetTypeId;

template <>
struct GetTypeId<int> {
    static const Type type() { return Type("int"); }
};

template <>
struct GetTypeId<std::string> {
    static const Type type() { return Type("str"); }
};

template <class... Args>
struct Mangler;

template <>
struct Mangler<> {
    static const std::string Mangle() { return ""; }
};

template <class A, class... Args>
struct Mangler<A, Args...> {
    static const std::string Mangle() {
        return "@" + GetTypeId<A>::type().name + Mangler<Args...>::Mangle();
    }
};

template <class F>
struct ManglerCaller : public ManglerCaller<decltype(&F::operator())> {};

template <class R, class... Args>
struct ManglerCaller<R (*)(Args...)> {
    static const std::string Mangle() { return Mangler<Args...>::Mangle(); }
    static const std::string Result() { return GetTypeId<R>::type(); }
};

template <class R, class C, class... Args>
struct ManglerCaller<R (C::*)(Args...) const> {
    static const std::string Mangle() { return Mangler<Args...>::Mangle(); }
    static const std::string Result() { return GetTypeId<R>::type().name; }
};

struct Function {
    std::string mangled_name;
    Type return_type;
};

struct CompilationContext {
    std::map<std::string, Function> functions_;

    template <class F>
    void DeclareFun(const std::string& name, F) {
        // FIXME: the func ptr isn't stored because I can't think of a way to
        // retrieve its type now.
        Function f;
        f.mangled_name = name + ManglerCaller<F>::Mangle();
        f.return_type = ManglerCaller<F>::Result();
        functions_[f.mangled_name] = f;
        std::cout << "adding " << f.mangled_name << "\n";
    }
};

class TypeChecker : public Visitor {
    Type ret_;
    CompilationContext& ctx_;

   public:
    TypeChecker(CompilationContext& ctx) : ctx_(ctx) {}

    void Visit(slip::Int&) override { ret_ = Type("int"); }
    void Visit(slip::Atom&) override {
        throw std::runtime_error("not implemented yet");
    }
    void Visit(slip::Str&) override { ret_ = Type("str"); }
    void Visit(slip::List& xs) override {
        if (xs.empty()) {
            ret_ = Type("void");
            return;
        }

        std::string* fun_name = xs.GetFunName();
        if (!fun_name) {
            ret_ = Type("void");
            return;
        }

        for (size_t i = 1; i < xs.size(); ++i) {
            xs[i]->Accept(*this);
            *fun_name += "@" + ret_.name;
        }

        auto found = ctx_.functions_.find(*fun_name);
        if (found == ctx_.functions_.end()) {
            throw std::runtime_error("Can't resolve overload " + *fun_name);
        }
        ret_ = found->second.return_type;
    }
};

void TypeCheck(Val& x, CompilationContext& ctx) {
    TypeChecker tc(ctx);
    tc(x);
}
}  // namespace slip

#pragma once

#include "ast.h"

namespace slip {
namespace {
class PrintVisitor : public slip::ConstVisitor {
    std::string res_;

   public:
    void Visit(const slip::Int& x) override {
        res_ += std::to_string(x.val()) + ":int";
    }
    void Visit(const slip::Bool& x) override {
        res_ += std::to_string(x.val()) + ":bool";
    }
    void Visit(const slip::Atom& x) override { res_ += x.val() + ":atom"; }
    void Visit(const slip::Str& x) override {
        res_ += "\"" + x.val() + "\":str";
    }
    void Visit(const slip::List& xs) override {
        res_ += "[";
        for (auto& x : xs) {
            x->Accept(*this);
            res_ += " ";
        }
        if (res_.back() == ' ') {
            res_.back() = ']';
        } else {
            res_.push_back(']');
        }
    }

    std::string result() const { return res_; }
};
}

inline std::string Print(const Val& v) {
    PrintVisitor pv;
    pv(v);
    return pv.result();
}

}  // namespace slip

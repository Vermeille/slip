#pragma once

#include "ast.h"

namespace slip {
namespace {
class PrintVisitor : public slip::Visitor {
    std::string res_;

   public:
    void Visit(slip::Int& x) override {
        res_ += std::to_string(x.val()) + ":int";
    }
    void Visit(slip::Atom& x) override { res_ += x.val() + ":atom"; }
    void Visit(slip::Str& x) override { res_ += "\"" + x.val() + "\":str"; }
    void Visit(slip::List& xs) override {
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

inline std::string Print(Val& v) {
    PrintVisitor pv;
    pv(v);
    return pv.result();
}

}  // namespace slip

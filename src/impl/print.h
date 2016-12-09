#pragma once

#include "ast.h"

namespace slip {
namespace {
class PrintVisitor : public boost::static_visitor<> {
    std::string res_;

   public:
    void operator()(const Int& x) { res_ += std::to_string(x.val()) + ":int"; }
    void operator()(const Bool& x) {
        res_ += std::to_string(x.val()) + ":bool";
    }
    void operator()(const Atom& x) { res_ += x.val() + ":atom"; }
    void operator()(const Str& x) { res_ += "\"" + x.val() + "\":str"; }
    void operator()(const List& xs) {
        res_ += "[";
        for (auto& x : xs) {
            boost::apply_visitor(*this, x);
            res_ += " ";
        }
        if (res_.back() == ' ') {
            res_.back() = ']';
        } else {
            res_.push_back(']');
        }
    }

    std::string result() { return res_; }
};
}

inline std::string Print(const Val& v) {
    PrintVisitor pv;
    boost::apply_visitor(pv, v);
    return pv.result();
}

}  // namespace slip

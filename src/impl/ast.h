#pragma once

#include <memory>
#include <string>
#include <vector>

#include <boost/variant.hpp>

namespace slip {
struct Int {
   public:
    Int() = default;
    Int(int i) : val_(i) {}
    Int(const Int& x) = default;
    int val() const { return val_; }

   private:
    int val_;
};

struct Bool {
   public:
    Bool(int i) : val_(i) {}
    Bool(const Bool& x) = default;
    bool val() const { return val_; }

   private:
    bool val_;
};

struct Atom {
   public:
    Atom(std::string s) : val_(s) {}
    Atom(const Atom& x) = default;
    const std::string& val() const { return val_; }
    std::string& val() { return val_; }

   private:
    std::string val_;
};

struct Str {
   public:
    Str(std::string s) : val_(std::move(s)) {}
    Str(const Str& x) = default;
    const std::string& val() const { return val_; }
    std::string& val() { return val_; }

   private:
    std::string val_;
};

class List;
using Val =
    boost::variant<Int, Bool, Atom, Str, boost::recursive_wrapper<List>>;

class List {
   public:
    List(std::vector<Val> v) : vals_(std::move(v)) {}

    const std::string* GetFunName() const {
        if (vals_.empty()) {
            return nullptr;
        }

        if (const Atom* a = boost::get<Atom>(&vals_[0])) {
            return &a->val();
        } else {
            return nullptr;
        }
    }
    decltype(auto) begin() { return vals_.begin(); }
    decltype(auto) begin() const { return vals_.begin(); }
    decltype(auto) end() { return vals_.end(); }
    decltype(auto) end() const { return vals_.end(); }
    bool empty() const { return vals_.empty(); }
    size_t size() const { return vals_.size(); }
    decltype(auto) operator[](size_t i) { return vals_[i]; }
    decltype(auto) operator[](size_t i) const { return vals_[i]; }

   private:
    std::vector<Val> vals_;
};
}  // namespace slip

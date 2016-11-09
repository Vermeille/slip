#pragma once

#include <memory>
#include <string>
#include <vector>

namespace slip {
class Visitor;

struct Val {
    virtual ~Val(){};
    virtual void Accept(Visitor&);
};

struct Int : public Val {
   public:
    Int(int i) : val_(i) {}
    Int(const Int& x) = default;
    virtual void Accept(Visitor&) override;
    int val() const { return val_; }

   private:
    int val_;
};

struct Atom : public Val {
   public:
    Atom(std::string s) : val_(s) {}
    Atom(const Atom& x) = default;
    virtual void Accept(Visitor&) override;
    const std::string& val() const { return val_; }
    std::string& val() { return val_; }

   private:
    std::string val_;
};

struct Str : public Val {
   public:
    Str(std::string s) : val_(std::move(s)) {}
    Str(const Str& x) = default;
    virtual void Accept(Visitor&) override;
    const std::string& val() const { return val_; }
    std::string& val() { return val_; }

   private:
    std::string val_;
};

struct List : public Val {
   public:
    List(std::vector<std::unique_ptr<Val>> v) : vals_(std::move(v)) {}
    List(List&&) = default;
    virtual void Accept(Visitor&) override;
    std::string* GetFunName() const {
        if (vals_.empty()) {
            return nullptr;
        }

        if (Atom* a = dynamic_cast<Atom*>(vals_[0].get())) {
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
    decltype(auto) operator[](int i) { return vals_[i]; }
    decltype(auto) operator[](int i) const { return vals_[i]; }

   private:
    std::vector<std::unique_ptr<Val>> vals_;
};

class Visitor {
   public:
    virtual void Visit(Int& x) = 0;
    virtual void Visit(Atom& x) = 0;
    virtual void Visit(Str& x) = 0;
    virtual void Visit(List& x) = 0;
    virtual void Visit(Val& x) { x.Accept(*this); }
    void operator()(Val& x) { x.Accept(*this); }
    ~Visitor() {}
};

void Int::Accept(Visitor& v) { v.Visit(*this); }
void Atom::Accept(Visitor& v) { v.Visit(*this); }
void Str::Accept(Visitor& v) { v.Visit(*this); }
void List::Accept(Visitor& v) { v.Visit(*this); }
void Val::Accept(Visitor& v) { v.Visit(*this); }

}  // namespace slip

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
    int val;
    Int(int i) : val(i) {}
    Int(const Int& x) = default;
    virtual void Accept(Visitor&) override;
};

struct Atom : public Val {
    std::string val;
    Atom(std::string s) : val(s) {}
    Atom(const Atom& x) = default;
    virtual void Accept(Visitor&) override;
};

struct List : public Val {
    std::vector<std::unique_ptr<Val>> vals;

    List(std::vector<std::unique_ptr<Val>> v) : vals(std::move(v)) {}
    List(List&&) = default;
    virtual void Accept(Visitor&) override;
    std::string* GetFunName() const {
        if (vals.empty()) {
            return nullptr;
        }

        if (Atom* a = dynamic_cast<Atom*>(vals[0].get())) {
            return &a->val;
        } else {
            return nullptr;
        }
    }
};

class Visitor {
   public:
    virtual void Visit(Int& x) = 0;
    virtual void Visit(Atom& x) = 0;
    virtual void Visit(List& x) = 0;
    virtual void Visit(Val& x) { x.Accept(*this); }
    void operator()(Val& x) { x.Accept(*this); }
    ~Visitor() {}
};

void Int::Accept(Visitor& v) { v.Visit(*this); }
void Atom::Accept(Visitor& v) { v.Visit(*this); }
void List::Accept(Visitor& v) { v.Visit(*this); }
void Val::Accept(Visitor& v) { v.Visit(*this); }

}  // namespace slip

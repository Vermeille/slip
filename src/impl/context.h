#pragma once

#include <iostream>
#include <map>
#include <memory>
#include <string>

#include "function.h"
#include "mangler.h"

namespace slip {
class Context {
   public:
    Context() = default;
    Context(const Context&) = delete;

    template <class F>
    void DeclareFun(std::string name, F&& f);

    template <class F>
    void DeclareFun(std::string name, std::string type, F&& f);

    template <class F>
    void DeclareSpecial(std::string name, std::string ty, F&& f);

    Function* Find(const std::string& name) const;

    void Dump() const;

    void ImportBase();

   private:
    std::map<std::string, std::unique_ptr<Function>> functions_;
};
}  // namespace slip

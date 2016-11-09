#pragma once

#include <iostream>
#include <map>
#include <memory>
#include <string>

#include "function.h"
#include "mangler.h"

namespace slip {
class Context {
    std::map<std::string, std::unique_ptr<Function>> functions_;

   public:
    template <class F>
    void DeclareFun(std::string name, F&& f) {
        auto ptr = std::unique_ptr<Function>(
            new FunctionImpl<typename ManglerCaller<F>::result_type>(
                std::move(name), std::move(f)));
        functions_[ptr->mangled_name] = std::move(ptr);
    }

    Function* Find(const std::string& name) const {
        auto found = functions_.find(name);
        if (found != functions_.end()) {
            return found->second.get();
        }
        return nullptr;
    }

    void Dump() const {
        for (auto& x : functions_) {
            std::cout << x.second->mangled_name << " => "
                      << x.second->return_type.name << "\n";
        }
    }
};
}  // namespace slip

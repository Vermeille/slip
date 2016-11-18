#pragma once

#include <cassert>
#include <map>
#include <memory>
#include <sstream>

namespace slip {
namespace experimental {

struct TypeBase {
    using BoundVars = std::map<std::string, const TypeBase*>;
    enum class From { Left, Right };

    virtual void Show(From, std::ostream& os) const = 0;
    virtual bool Match(const TypeBase&) const = 0;
    virtual std::unique_ptr<TypeBase> Substitute(const BoundVars&) const = 0;
    virtual void Bind(BoundVars&, const TypeBase&) const = 0;
    virtual std::unique_ptr<TypeBase> Clone() const = 0;
};

struct AtomType : public TypeBase {
    std::string nm_;

    AtomType(std::string nm) : nm_(std::move(nm)) {}

    virtual void Show(From, std::ostream& os) const { os << nm_; }

    virtual bool Match(const TypeBase& x) const {
        if (IsVar()) {
            return true;
        }

        if (const AtomType* y = dynamic_cast<const AtomType*>(&x)) {
            return y->nm_ == nm_;
        }
        return false;
    }

    virtual std::unique_ptr<TypeBase> Clone() const {
        return std::make_unique<AtomType>(nm_);
    }

    bool IsVar() const { return islower(nm_[0]); }

    virtual void Bind(BoundVars& vars, const TypeBase& x) const {
        if (!IsVar()) {
            const AtomType* y = dynamic_cast<const AtomType*>(&x);
            assert(y && y->nm_ == nm_);
            return;
        }

        auto inserted = vars.insert(std::make_pair(nm_, &x));
        if (!inserted.second) {
            throw std::runtime_error(nm_ + " already bound");
        }
    }

    virtual std::unique_ptr<TypeBase> Substitute(const BoundVars& vars) const {
        if (!IsVar()) {
            return std::make_unique<AtomType>(nm_);
        }

        auto found = vars.find(nm_);
        if (found != vars.end()) {
            return found->second->Clone();
        }

        return Clone();
    }
};

struct FunctionType : public TypeBase {
    std::unique_ptr<TypeBase> args_;
    std::unique_ptr<TypeBase> ret_;

    FunctionType() = default;

    template <class T, class U>
    FunctionType(T&& x, U&& y)
        : args_(new T(std::forward<T>(x))), ret_(new U(std::forward<U>(y))) {}

    virtual void Show(From f, std::ostream& os) const {
        if (f == From::Right) {
            os << "(";
        }
        args_->Show(From::Right, os);
        os << " -> ";
        ret_->Show(From::Left, os);
        if (f == From::Right) {
            os << ")";
        }
    }

    virtual bool Match(const TypeBase& x) const {
        if (const FunctionType* y = dynamic_cast<const FunctionType*>(&x)) {
            return args_->Match(*y->args_) && ret_->Match(*y->ret_);
        }
        return false;
    }

    virtual std::unique_ptr<TypeBase> Clone() const {
        FunctionType* f = new FunctionType;
        f->args_ = args_->Clone();
        f->ret_ = ret_->Clone();
        return std::unique_ptr<TypeBase>(f);
    }

    virtual void Bind(BoundVars& vars, const TypeBase& x) const {
        if (const FunctionType* y = dynamic_cast<const FunctionType*>(&x)) {
            args_->Bind(vars, *y->args_);
            ret_->Bind(vars, *y->ret_);
        } else {
            throw std::runtime_error(
                "FunctionType doesn't match with concrete");
        }
    }

    virtual std::unique_ptr<TypeBase> Substitute(const BoundVars& vars) const {
        FunctionType* f = new FunctionType;
        f->args_ = args_->Substitute(vars);
        f->ret_ = ret_->Substitute(vars);
        return std::unique_ptr<TypeBase>(f);
    }

    std::unique_ptr<TypeBase> Apply(const TypeBase& x) const {
        if (!args_->Match(x)) {
            return nullptr;
        }
        BoundVars vars;
        args_->Bind(vars, x);
        return ret_->Substitute(vars);
    }
};

class Type {
   public:
    template <class T>
    Type(T&& x) : ty_(new T(std::forward<T>(x))) {}

    template <class T>
    Type(std::unique_ptr<T>&& ptr) : ty_(std::move(ptr)) {}

    std::string Show() const {
        std::ostringstream oss;
        ty_->Show(TypeBase::From::Left, oss);
        return oss.str();
    };

    bool IsValid() const { return ty_ != nullptr; }

    Type Apply(const Type& ty) const {
        auto fun = dynamic_cast<const FunctionType*>(ty_.get());
        if (!fun) {
            throw std::runtime_error(Show() + " is not a function");
        }
        auto res = fun->Apply(*ty.ty_);
        if (!res) {
            throw std::runtime_error("Something went wrong: apply " +
                                     ty.Show() + " to " + Show());
        }
        return std::move(res);
    }

   private:
    std::unique_ptr<TypeBase> ty_;
};

}  // namespace experimental
}  // namespace slip

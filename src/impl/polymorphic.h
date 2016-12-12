#pragma once

#include <memory>
#include <sstream>

#include "detect_trait.h"

template <class T>
using has_to_string = decltype(std::to_string(std::declval<T>()));

template <class T>
struct ShowImpl {
    static std::string Show(const T& x) { return std::to_string(x); }
};

template <class T>
using has_ostream =
    decltype(std::declval<std::ostringstream&>() << std::declval<T>());

template <class T>
struct ShowOstreamImpl {
    static std::string Show(const T& x) {
        std::ostringstream oss;
        oss << x;
        return oss.str();
    }
};

template <class T>
using has_show = decltype(std::declval<T>().Show());

template <class T>
struct ShowMemberImpl {
    static std::string Show(const T& x) { return x.Show(); }
};

template <class T>
struct PrintError {
    static std::string Show(const T& x) {
        throw std::runtime_error(std::string("No print for ") +
                                 typeid(x).name());
    }
};

template <class T>
struct Shower
    : public std::conditional<
          Detect<has_to_string, T>::value,
          ShowImpl<T>,
          typename std::conditional<
              Detect<has_ostream, T>::value,
              ShowOstreamImpl<T>,
              typename std::conditional<Detect<has_show, T>::value,
                                        ShowMemberImpl<T>,
                                        PrintError<T>>::type>::type>::type {};

namespace slip {
class Polymorphic {
   public:
    template <class T>
    Polymorphic(T x)
        : value_(std::make_unique<Polymorphic_<T>>(std::move(x))) {}

    Polymorphic() = default;

    template <class T>
    std::enable_if_t<std::is_pointer<T>::value, T> as() const;

    template <class T>
    std::enable_if_t<!std::is_pointer<T>::value, T> as() const;

    Polymorphic(const Polymorphic& x) : value_(x.value_->Copy()) {}

    Polymorphic(Polymorphic&& x) : value_(std::move(x.value_)) {}

    std::string Show() { return value_->Show(); }

    Polymorphic& operator=(Polymorphic&&) = default;

   private:
    class PolymorphBase {
       public:
        virtual PolymorphBase* Copy() const = 0;
        virtual std::string Show() const = 0;
    };

    template <class T>
    class Polymorphic_ : public PolymorphBase, public Shower<T> {
       public:
        Polymorphic_(T x) : value_(std::move(x)) {}

        const T& value() const { return value_; }
        T& value() { return value_; }

        virtual Polymorphic_<T>* Copy() const override {
            return new Polymorphic_<T>(value_);
        }

        virtual std::string Show() const override {
            return Shower<T>::Show(value_);
        }

       private:
        T value_;
    };
    std::unique_ptr<PolymorphBase> value_;
};

template <class T>
std::enable_if_t<std::is_pointer<T>::value, T> Polymorphic::as() const {
    using t_no_ptr = std::remove_pointer_t<std::decay_t<T>>;
    auto ptr = dynamic_cast<Polymorphic_<t_no_ptr>*>(value_.get());
    if (!ptr) {
        return nullptr;
    }
    return &ptr->value();
}

template <class T>
std::enable_if_t<!std::is_pointer<T>::value, T> Polymorphic::as() const {
    auto ptr = as<T*>();
    if (!ptr) {
        throw std::bad_cast();
    }
    return *ptr;
}
}  // namespace slip

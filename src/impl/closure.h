#pragma once

#include <array>
#include <utility>

#include "ast.h"
#include "mangler.h"
#include "polymorphic.h"

namespace slip {
template <int>
class Number {};

class Context;

template <class T>
T Eval(const Val& x, Context& ctx);

class ClosureBase {
   public:
    virtual void Apply(const Val& x, Context& ctx) = 0;
    virtual Polymorphic GetResult() const = 0;
    virtual bool IsTotallyApplied() const = 0;
    virtual ClosureBase* Copy() const = 0;
    virtual std::string Show() const = 0;
    virtual ~ClosureBase() {}
};

template <class F>
class ClosureImpl : public ClosureBase {
   public:
    virtual Polymorphic GetResult() const override {
        if (filled_args_ != arity_) {
            throw std::runtime_error(std::to_string(filled_args_) +
                                     " arguments provided, but " +
                                     std::to_string(arity_) + " expected");
        }
        return Call(std::make_index_sequence<arity_>());
    }

    void Apply(const Val& x, Context& ctx) override {
        ApplyImpl(x, ctx, Number<0>());
        ++filled_args_;
    }

    bool IsTotallyApplied() const override { return filled_args_ == arity_; }

    ClosureImpl(std::string name, F f)
        : f_(std::move(f)), filled_args_(0), name_(std::move(name)) {}
    ClosureImpl(const ClosureImpl&) = default;

    ClosureBase* Copy() const override { return new ClosureImpl<F>(*this); }

   private:
    static constexpr int arity_ = ManglerCaller<std::decay_t<F>>::arity;

    template <int N>
    void ApplyImpl(const Val& x, Context& ctx, Number<N>) {
        if (N == filled_args_) {
            std::get<N>(args_) =
                Eval<typename std::tuple_element<N, decltype(args_)>::type>(
                    x, ctx);
        } else {
            ApplyImpl(x, ctx, Number<N + 1>());
        }
    }

    void ApplyImpl(const Val&, Context&, Number<arity_>) {
        throw std::runtime_error("No more remaining unfilled arguments");
    }

    using args_type = typename ManglerCaller<std::decay_t<F>>::args_type;

    template <size_t... Ns>
    auto Call(std::index_sequence<Ns...>) const {
        return f_(std::get<Ns>(args_)...);
    }

    std::string Show() const override { return name_ + ShowArgs(Number<0>()); }

    template <int N>
    std::string ShowArgs(Number<N>) const {
        if (N < filled_args_) {
            return " (" + (Polymorphic(std::get<N>(args_))).Show() + ")" +
                   ShowArgs(Number<N + 1>());
        } else {
            return " _" + ShowArgs(Number<N + 1>());
        }
    }

    std::string ShowArgs(Number<arity_>) const { return ""; }

    F f_;
    args_type args_;
    int filled_args_ = 0;
    std::string name_;
};

template <class F>
class SpecialClosureImpl : public ClosureBase {
    static constexpr int arity_ = std::tuple_size<
        std::tuple_element_t<0, typename ManglerCaller<F>::args_type>>::value;

   public:
    virtual Polymorphic GetResult() const override {
        if (filled_args_ != arity_) {
            throw std::runtime_error(std::to_string(filled_args_) +
                                     " arguments provided, but " +
                                     std::to_string(arity_) + " expected");
        }
        return f_(args_);
    }

    void Apply(const Val& x, Context& ctx) override {
        args_[filled_args_] = std::make_pair(&x, &ctx);
        ++filled_args_;
    }

    bool IsTotallyApplied() const override { return filled_args_ == arity_; }

    SpecialClosureImpl(std::string nm, F f)
        : f_(std::move(f)), filled_args_(0), name_(std::move(nm)) {}
    SpecialClosureImpl(const SpecialClosureImpl&) = default;

    ClosureBase* Copy() const override {
        return new SpecialClosureImpl<F>(*this);
    }

    std::string Show() const override {
        std::ostringstream oss;
        oss << name_;
        for (size_t i = 0; i < args_.size(); ++i) {
            if (i < filled_args_) {
                oss << " (" << Print(*args_[i].first) << ")";
            } else {
                oss << " _";
            }
        }
        return oss.str();
    }

   private:
    F f_;
    std::array<std::pair<const Val*, Context*>, arity_> args_;
    size_t filled_args_;
    std::string name_;
};

class Closure {
   public:
    template <class F>
    static Closure Get(std::string nm, F&& f) {
        return Closure(std::make_unique<ClosureImpl<std::decay_t<F>>>(
            std::move(nm), std::move(f)));
    }
    template <class F>
    static Closure GetSpecial(std::string nm, F&& f) {
        return Closure(std::make_unique<SpecialClosureImpl<std::decay_t<F>>>(
            std::move(nm), std::move(f)));
    }

    void Apply(const Val& x, Context& ctx) { return base_->Apply(x, ctx); }

    template <class R>
    R GetResult() const {
        return base_->GetResult().as<R>();
    }

    Polymorphic GetResult() const { return base_->GetResult(); }

    bool IsTotallyApplied() const { return base_->IsTotallyApplied(); }

    Closure(const Closure& c) : base_(c.base_->Copy()) {}
    Closure(Closure&& c) = default;

    Closure& operator=(Closure&&) = default;

    std::string Show() const { return base_->Show(); }

   private:
    Closure(std::unique_ptr<ClosureBase> clo) : base_(std::move(clo)) {}
    std::unique_ptr<ClosureBase> base_;
};

}  // namespace slip

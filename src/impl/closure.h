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

    bool IsTotallyApplied() const { return filled_args_ == arity_; }

    ClosureImpl(F&& f) : f_(std::move(f)) {}

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

    F f_;
    args_type args_;
    int filled_args_ = 0;
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

    bool IsTotallyApplied() const { return filled_args_ == arity_; }

    SpecialClosureImpl(F&& f) : f_(std::move(f)) {}

   private:
    F f_;
    std::array<std::pair<const Val*, Context*>, arity_> args_;
    int filled_args_ = 0;
};

class Closure {
   public:
    template <class F>
    static Closure Get(F&& f) {
        return Closure(new ClosureImpl<std::decay_t<F>>(std::move(f)));
    }
    template <class F>
    static Closure GetSpecial(F&& f) {
        return Closure(new SpecialClosureImpl<std::decay_t<F>>(std::move(f)));
    }

    void Apply(const Val& x, Context& ctx) { return base_->Apply(x, ctx); }

    template <class R>
    R GetResult() const {
        return base_->GetResult().as<R>();
    }

    Polymorphic GetResult() const { return base_->GetResult(); }

   private:
    Closure(ClosureBase* clo) : base_(clo) {}
    std::unique_ptr<ClosureBase> base_;
};

}  // namespace slip

#include <utility>

#include "ast.h"
#include "context.h"
#include "eval.h"
#include "mangler.h"

namespace slip {
template <int>
class Number {};

template <class T>
T Eval(const Val& x, Context& ctx);

template <class R>
class ClosureBase {
    virtual void Apply(const Val& x, Context& ctx) = 0;
    virtual R GetResult() const override = 0;
};

template <class F>
class ClosureImpl : public ClosureBase<std::result_of_t<F>> {
   public:
    virtual std::result_of_t<F> GetResult() const override {
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
    static constexpr int arity_ = Mangler<F>::arity;

    template <int N>
    void ApplyImpl(const Val& x, Context& ctx, Number<N>) {
        if (N == filled_args_) {
            std::get<N>(args_) =
                Eval<typename std::tuple_element<N, decltype(args_)>::type>(
                    x, ctx);
        } else {
            app_(x, Number<N + 1>());
        }
    }

    void ApplyImpl(const Val&, Context&, Number<arity_>) {
        throw std::runtime_error("No more remaining unfilled arguments");
    }

    using args_type = typename Mangler<F>::args_type;

    template <size_t... Ns>
    auto Call(std::index_sequence<Ns...>) {
        return f_(std::get<Ns>(args_)...);
    }

    F f_;
    args_type args_;
    int filled_args_ = 0;
};

template <class R>
class Closure {
   public:
    template <class F>
    Closure(F&& f) : base_(new ClosureImpl<F>(std::move(f))) {}

    void Apply(const Val& x, Context& ctx) { return base_->Apply(x, ctx); }

    R GetResult() const { return base_->GetResult(); }

   private:
    std::unique_ptr<ClosureBase<R>> base_;
};

}  // namespace slip

#pragma once
static_assert(__cplusplus >= 202300L, "cxx-libs requires C++23");
// (c) 2024 Steve O'Brien -- MIT License

#include "_Ref.h"

#include <cassert>
#include <coroutine>
#include <cstddef>
#include <exception>
#include <ranges>

namespace cxx {

template <typename T>
class Generator;

template <typename T>
struct Promise {
    T val_ {};
    std::exception_ptr exc_;

    ~Promise() noexcept = default;

    std::suspend_always initial_suspend() noexcept { return {}; }
    std::suspend_always final_suspend() noexcept { return {}; }
    void return_void() noexcept {}
    void unhandled_exception() noexcept { exc_ = std::current_exception(); }
    Generator<T> get_return_object() noexcept { return {this}; }

    std::suspend_always yield_value(T val) noexcept {
        val_ = val;
        return {};
    }
};

template <typename T>
struct Coro final {
    using Handle = std::coroutine_handle<Promise<T>>;
    Handle handle;
    ~Coro() noexcept { handle.destroy(); }
    Coro(Promise<T>* promise) noexcept : handle(Handle::from_promise(*promise)) {}
    virtual bool done() const noexcept { return handle.done(); }
};

template <typename T>
struct CoroIteratorBase {
    using value_type = T;
    using difference_type = int;

    ~CoroIteratorBase() noexcept = default;
    CoroIteratorBase() noexcept = default;

    virtual bool done() const = 0;
    bool operator==(CoroIteratorBase const& rhs) const noexcept { return done() == rhs.done(); }
};

template <typename T>
struct CoroIterator final : CoroIteratorBase<T> {
    Ref<Coro<T>> coro_;

    ~CoroIterator() noexcept = default;
    CoroIterator() noexcept = default;
    CoroIterator(Ref<Coro<T>> coro) noexcept : coro_(std::move(coro)) {}
    CoroIterator(CoroIterator const&) noexcept = default;
    CoroIterator& operator=(CoroIterator const&) noexcept = default;

    virtual bool done() const noexcept { return coro_->done(); }

    T operator*() const noexcept {
        auto& p = coro_->handle.promise();
        if (p.exc_) { std::rethrow_exception(p.exc_); }
        return p.val_;
    }

    CoroIterator<T>& operator++(int) noexcept { return this->operator++(); }
    CoroIterator<T>& operator++() noexcept {
        coro_->handle.resume();
        return *this;
    }
};

template <typename T>
struct CoroEndIterator final : CoroIteratorBase<T> {
    CoroEndIterator() noexcept = default;
    CoroEndIterator(CoroEndIterator const&) noexcept = default;
    virtual bool done() const noexcept { return true; }
};

template <typename T>
class Generator final : std::ranges::view_interface<Generator<T>> {
    Ref<Coro<T>> coro_ {};

public:
    using value_type = T;
    using promise_type = Promise<T>;
    using handle_type = Coro<T>;

    ~Generator() noexcept = default;
    Generator() noexcept = default;
    Generator(Generator<T> const&) noexcept = default;
    Generator(Generator<T>&&) noexcept = default;
    Generator& operator=(Generator<T> const&) noexcept = default;
    Generator& operator=(Generator<T>&&) noexcept = default;

    Generator(promise_type* promise) : coro_(cxx::Ref<Coro<T>>::make(promise)) {}

    CoroIterator<T> begin(this auto& self) { return ++(CoroIterator<T> {self.coro_}); }
    CoroEndIterator<T> end(this auto&) { return CoroEndIterator<T>(); }

    CoroIterator<T> cbegin() const { return begin(); }
    CoroEndIterator<T> cend() const { return end(); }
};

}  // namespace cxx

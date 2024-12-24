#pragma once
static_assert(__cplusplus >= 202300L, "cxx-libs requires C++23");
// (c) 2024 Steve O'Brien -- MIT License

#include "Ref.h"

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
    T val {};
    std::exception_ptr exc;
    std::suspend_always initial_suspend() { return {}; }
    std::suspend_always final_suspend() noexcept { return {}; }
    void return_void() {}
    void unhandled_exception() { exc = std::current_exception(); }
    Generator<T> get_return_object() { return {this}; }

    std::suspend_always yield_value(T val) {
        this->val = std::move(val);
        return {};
    }
};

template <typename T>
struct Coro final {
    using Handle = std::coroutine_handle<Promise<T>>;
    Handle handle;
    ~Coro() { handle.destroy(); }
    Coro(Promise<T>* promise) : handle(Handle::from_promise(*promise)) {}
    virtual bool done() const { return handle.done(); }
};

template <typename T>
struct CoroIteratorBase {
    using value_type = T;
    using difference_type = int;

    ~CoroIteratorBase() = default;
    CoroIteratorBase() = default;

    virtual bool done() const = 0;
    bool operator==(CoroIteratorBase const& rhs) const { return done() == rhs.done(); }
};

template <typename T>
struct CoroIterator final : CoroIteratorBase<T> {
    Ref<Coro<T>> coro_;

    CoroIterator() = default;
    CoroIterator(Ref<Coro<T>> coro) : coro_(coro) {}
    CoroIterator(CoroIterator const&) = default;
    CoroIterator& operator=(CoroIterator const&) = default;

    virtual bool done() const { return coro_->done(); }

    T operator*() const {
        auto& p = coro_->handle.promise();
        if (p.exc) { std::rethrow_exception(p.exc); }
        return p.val;
    }

    CoroIterator<T>& operator++(int) { return this->operator++(); }
    CoroIterator<T>& operator++() {
        coro_->handle.resume();
        return *this;
    }
};

template <typename T>
struct CoroEndIterator final : CoroIteratorBase<T> {
    CoroEndIterator() = default;
    CoroEndIterator(CoroEndIterator const&) = default;
    virtual bool done() const { return true; }
};

template <typename T>
class Generator final : std::ranges::view_interface<Generator<T>> {
    Ref<Coro<T>> coro_ {};

public:
    using value_type = T;
    using promise_type = Promise<T>;
    using handle_type = Coro<T>;

    ~Generator() = default;
    Generator() = default;
    Generator(Generator<T> const&) = default;
    Generator(Generator<T>&&) = default;
    Generator& operator=(Generator<T> const&) = default;
    Generator& operator=(Generator<T>&&) = default;

    Generator(promise_type* promise) : coro_(cxx::ref<Coro<T>>(promise)) {}

    CoroIterator<T> begin(this auto& self) { return ++(CoroIterator<T> {self.coro_}); }
    CoroEndIterator<T> end(this auto&) { return CoroEndIterator<T>(); }

    CoroIterator<T> cbegin() const { return begin(); }
    CoroEndIterator<T> cend() const { return end(); }
};

}  // namespace cxx

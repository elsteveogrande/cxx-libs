#pragma once
static_assert(__cplusplus >= 202300L, "cxx-libs requires C++23");
// (c) 2024 Steve O'Brien -- MIT License

#include "cxx/Ref.h"

#include <cassert>
#include <concepts>
#include <coroutine>
#include <cstddef>
#include <exception>
#include <ranges>

namespace cxx {

template <std::semiregular T>
struct Generator;

template <std::semiregular T>
struct Promise {
    T val;
    std::exception_ptr exc;
    std::suspend_always initial_suspend() { return {}; }
    std::suspend_always final_suspend() noexcept { return {}; }
    Generator<T> get_return_object() { return Generator(this); }
    void return_void() {}
    void unhandled_exception() { exc = std::current_exception(); }
    std::suspend_always yield_value(T val) {
        this->val = val;
        return {};
    }
};

template <std::semiregular T>
struct Coro final : RefCounted<Coro<T>> {
    using Handle = std::coroutine_handle<Promise<T>>;
    Handle handle;
    ~Coro() { handle.destroy(); }
    Coro(Promise<T>* promise) : handle(Handle::from_promise(*promise)) {}
    virtual bool done() const { return handle.done(); }
};

template <std::semiregular T>
struct CoroIteratorBase {
    using value_type = T;
    using difference_type = int;

    ~CoroIteratorBase() = default;
    CoroIteratorBase() = default;

    virtual bool done() const = 0;
    bool operator==(CoroIteratorBase const& rhs) const { return done() == rhs.done(); }
};

template <std::semiregular T>
struct CoroIterator final : CoroIteratorBase<T> {
    Ref<Coro<T>> coro_;

    CoroIterator() = default;
    CoroIterator(Ref<Coro<T>> coro) : coro_(coro) {}
    CoroIterator(CoroIterator const&) = default;
    CoroIterator& operator=(CoroIterator const&) = default;

    virtual bool done() const { return coro_->done(); }

    T operator*() const { return coro_->handle.promise().val; }

    CoroIterator<T>& operator++(int) { return this->operator++(); }
    CoroIterator<T>& operator++() {
        coro_->handle.resume();
        return *this;
    }
};

template <std::semiregular T>
struct CoroEndIterator final : CoroIteratorBase<T> {
    CoroEndIterator() = default;
    CoroEndIterator(CoroEndIterator const&) = default;
    virtual bool done() const { return true; }
};

struct GeneratorBase {};

template <std::semiregular T>
struct Generator final : std::ranges::view_interface<Generator<T>>, GeneratorBase {
    using promise_type = Promise<T>;
    using handle_type = Coro<T>;

    Ref<Coro<T>> coro_;

    ~Generator() = default;
    Generator() = default;

    explicit Generator(promise_type* promise) : coro_(cxx::make<Coro<T>>(promise)) {}

    Generator(Generator<T> const& rhs) : coro_(rhs.coro_) {}

    Generator& operator=(Generator<T> const& rhs) { coro_ = rhs.coro_; }

    CoroIterator<T> begin() { return (CoroIterator<T> {coro_})++; }
    CoroEndIterator<T> end() { return CoroEndIterator<T>(); }
    CoroIterator<T> begin() const { return (CoroIterator<T> {coro_})++; }
    CoroEndIterator<T> end() const { return CoroEndIterator<T>(); }
    CoroIterator<T> cbegin() const { return (CoroIterator<T> {coro_})++; }
    CoroEndIterator<T> cend() const { return CoroEndIterator<T>(); }
};
static_assert(std::semiregular<Generator<int>>);

/** Something that is an instance of `Generator<T>` */
template <typename G, typename T>
concept Generates = requires(G&& gen) { static_cast<Generator<T> const*>(&gen); };

/** Something that's a `Generator` but we don't care what type it generates. */
template <typename G>
concept GeneratesAny = requires { static_cast<GeneratorBase*>(new G); };

}  // namespace cxx

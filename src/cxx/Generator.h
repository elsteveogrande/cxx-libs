#pragma once
static_assert(__cplusplus >= 202300L, "cxx-libs requires C++23");
// (c) 2024 Steve O'Brien -- MIT License

#include "cxx/Ref.h"

#include <cassert>
#include <coroutine>
#include <cstddef>
#include <exception>
#include <functional>
#include <iterator>

namespace cxx {

template <typename T> struct Generator;

template <typename T> struct Promise {
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

template <typename T> struct Coro final : RefCounted<Coro<T>> {
    using Handle = std::coroutine_handle<Promise<T>>;
    Handle handle;
    ~Coro() { handle.destroy(); }
    Coro(Promise<T>* promise)
            : handle(Handle::from_promise(*promise)) {}
    bool done() { return handle.done(); }
};

template <typename T> struct CoroIteratorBase {
    using value_type = T;
    Ref<Coro<T>> coro;
    bool operator==(std::default_sentinel_t) { return coro->done(); }
};

template <typename T> struct ConstCoroIterator : CoroIteratorBase<T> {
    ConstCoroIterator<T>& operator++(int) { return this->operator++(); }
    ConstCoroIterator<T>& operator++() {
        this->coro->handle.resume();
        return *this;
    }
    T operator*() { return this->coro->handle.promise().val; }
};

template <typename T> struct CoroIterator : ConstCoroIterator<T> {
    CoroIterator<T>& operator++(int) { return this->operator++(); }
    CoroIterator<T>& operator++() {
        this->coro->handle.resume();
        return *this;
    }
    T operator*() { return this->coro->handle.promise().val; }
};

template <typename T> struct Generator final {
    using promise_type = Promise<T>;
    using handle_type = Coro<T>;
    using iterator = CoroIterator<T>;
    using const_iterator = ConstCoroIterator<T>;

    Ref<Coro<T>> coro_;

    ~Generator() {}

    explicit Generator(promise_type* promise)
            : coro_(cxx::make<Coro<T>>(promise)) {}

    Generator(Generator<T> const& rhs)
            : coro_(rhs.coro_) {}

    template <typename I, typename J, typename U = typename I::value_type>
    static Generator<typename I::value_type> of(I it, J end, U const& = U()) {
        for (; it != end; ++it) {
            co_yield *it;
        }
    }

    CoroIterator<T> begin() {
        coro_->handle.resume();
        return {coro_};
    }

    ConstCoroIterator<T> begin() const {
        coro_->handle.resume();
        return {coro_};
    }

    ConstCoroIterator<T> cbegin() const {
        coro_->handle.resume();
        return {coro_};
    }

    std::default_sentinel_t end() { return {}; }
    std::default_sentinel_t end() const { return {}; }
    std::default_sentinel_t cend() const { return {}; }

    template <typename U> Generator<U> map(U (*func)(T)) {
        return map(std::function<U(T)>(func));
    }

    template <typename U> Generator<U> map(std::function<U(T)> func) {
        return map(begin(), end(), func);
    }

    template <typename U>
    Generator<U> map(auto it, auto end, std::function<U(T)> func) {
        while (it != end) {
            co_yield func(*it);
            ++it;
        }
    }
};

}  // namespace cxx

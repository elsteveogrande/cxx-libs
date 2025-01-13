#pragma once
static_assert(__cplusplus >= 202300L, "cxx-libs requires C++23");
// (c) 2024 Steve O'Brien -- MIT License

#include "../Concepts.h"

#include <cassert>
#include <type_traits>
#include <utility>

namespace cxx {

/**
 * Similar to the standard C++23 `std::expected`, but differs in several ways.
 * Contains a union which has either the expected type `T` or an error object of type `E`.
 * The type parameter `E` is a type which would typically be thrown, e.g. an exception
 * (but this is not enforced).
 *
 * TODO: drop this when `std::expected` is available in libcxx.
 */

template <typename T, class E>
    requires DifferentRCV<T, E> struct Expected final {

    // For producing compatible `noexcept` signatures on dtor / move / copy functions.
    constexpr static bool kTNoThrowCopy = std::is_nothrow_copy_constructible_v<T>;
    constexpr static bool kTNoThrowMove = std::is_nothrow_move_constructible_v<T>;
    constexpr static bool kTNoThrowDtor = std::is_nothrow_destructible_v<T>;
    constexpr static bool kENoThrowCopy = std::is_nothrow_copy_constructible_v<E>;
    constexpr static bool kENoThrowMove = std::is_nothrow_move_constructible_v<E>;
    constexpr static bool kENoThrowDtor = std::is_nothrow_destructible_v<E>;

    // Whether the expected type is present (else error)
    bool const ok_;

    union Result {
        T t {};
        E e;
        ~Result() noexcept(kTNoThrowDtor && kENoThrowDtor) {}
    };
    Result result_;

    ~Expected() noexcept(kTNoThrowDtor && kENoThrowDtor) {
        if (ok_) {
            result_.t.~T();
        } else {
            result_.e.~E();
        }
    }

    Expected(T const& val) noexcept(kTNoThrowCopy) : ok_(true), result_ {.t = val} {};
    Expected(E const& val) noexcept(kENoThrowCopy) : ok_(false), result_ {.e = val} {};

    Expected(T&& val) noexcept(kTNoThrowMove) : ok_(true), result_ {.t = std::move(val)} {};
    Expected(E&& val) noexcept(kENoThrowMove) : ok_(false), result_ {.e = std::move(val)} {};

    operator bool() const { return ok_; }

    T* operator->() {
        if (!ok_) { throw result_.e; }
        return &result_.t;
    }

    T const* operator->() const {
        if (!ok_) { throw result_.e; }
        return &result_.t;
    }

    T& operator*() {
        if (!ok_) { throw result_.e; }
        return result_.t;
    }

    T const& operator*() const {
        if (!ok_) { throw result_.e; }
        return result_.t;
    }

    template <typename U>
    bool operator==(U const& u) const {
        return ok_ && result_.t == u;
    }
};

}  // namespace cxx

#pragma once
static_assert(__cplusplus >= 202300L, "cxx-libs requires C++23");
// (c) 2024 Steve O'Brien -- MIT License

#include "Exception.h"

#include <atomic>
#include <cassert>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace cxx::detail {

class RefCountedBase {
private:
    std::atomic_int64_t mutable rc_;

public:
    virtual ~RefCountedBase() = default;
    inline void inc() const { ++rc_; }
    inline bool dec() const { return --rc_ == -1; }
    int64_t count() const { return rc_; }
};

}  // namespace cxx::detail

namespace cxx {

struct RefBase {};

template <class T>
class Ref final : public RefBase {

protected:
    detail::RefCountedBase const* obj_ {nullptr};

public:
    struct NullRef final : Exception {};

    constexpr void clear() noexcept(true) {
        if (!obj_) { return; }
        if (obj_->dec()) { delete obj_; }
        obj_ = nullptr;
    }

    constexpr ~Ref() noexcept(true) { clear(); }

    constexpr Ref() = default;

    constexpr Ref(T* obj) noexcept(true) : obj_(obj) {
        // obj's `rc` is zero-initialized, which indicates
        // a reference count of 1; don't increment it.
    }

    constexpr Ref(Ref const& rhs) noexcept(true) : obj_(rhs.get()) {
        if (obj_) { obj_->inc(); }
    }

    constexpr Ref& operator=(Ref const& rhs) noexcept(true) {
        clear();
        new (this) Ref(rhs);
        return *this;
    }

    constexpr Ref(Ref&& rhs) noexcept(true) : obj_(rhs.get()) {
        // Steal this obj pointer, clear it in rhs.
        // Avoids the needs to increment / decrement refcount.
        rhs.obj_ = nullptr;
    }

    constexpr Ref& operator=(Ref&& rhs) noexcept(true) {
        // Steal this obj pointer, clear it in rhs.
        // Avoids the needs to increment / decrement refcount.
        this->obj_ = rhs.obj_;
        rhs.obj_ = nullptr;
        return *this;
    }

    template <typename U>
        requires(!std::is_same_v<std::remove_cv_t<U>, std::remove_cv_t<T>>)
    constexpr operator Ref<U>() {
        U* obj = (U*) obj_;
        if (obj) { obj->inc(); }
        return Ref<U>(obj);
    }

    constexpr T* get() noexcept(true) { return (T*) obj_; }
    constexpr T const* get() const noexcept(true) { return (T const*) obj_; }

    constexpr T* operator->() noexcept(true) { return get(); }
    constexpr T const* operator->() const noexcept(true) { return get(); }

    constexpr T& operator*() {
        if (!obj_) { throw NullRef(); }
        return *obj_;
    }

    constexpr T const& operator*() const {
        if (!obj_) { throw NullRef(); }
        return *obj_;
    }
};
static_assert(std::semiregular<Ref<int>>);
static_assert(sizeof(Ref<int>) == sizeof(uintptr_t));

template <typename T>
class RefCounted : public detail::RefCountedBase {
protected:
    virtual ~RefCounted() = default;
};

template <typename U, typename... A>
static Ref<U> make(A&&... args) {
    U* ptr = new U(args...);
    return Ref<U>(ptr);
}

}  // namespace cxx

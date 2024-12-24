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

template <typename T>
class Ref;

template <typename T>
struct RefCountBlock {
    std::atomic_int64_t mutable refs_ {0};
    union {
        char dummyObject {};  // to prevent `obj_` default-init
        T obj_;
    };

    RefCountBlock() {}
    ~RefCountBlock() { obj_.~T(); }
};

template <typename T>
struct RefControl final : RefCountBlock<T> {
    inline RefControl* retain() {
        ++this->refs_;
        return this;
    }

    inline void release() {
        if (--this->refs_ == -1) { delete this; }
    }

    template <typename... A>
    RefControl(A&&... args) {
        new (&this->obj_) T(args...);
    }
};

}  // namespace cxx::detail

namespace cxx {

template <class T>
struct Ref final {
    detail::RefControl<T>* rc_ {nullptr};

    struct NullRef final : Exception {
        template <typename U>
        static U* check(U* ptr) {
            if (!ptr) { throw NullRef(); }
            return ptr;
        }
    };

    constexpr ~Ref() noexcept(true) {
        if (rc_) { rc_->release(); }
        rc_ = nullptr;
    }

    constexpr Ref() = default;

    constexpr Ref(Ref const& rhs) noexcept(true) : rc_(rhs.rc_->retain()) {}
    constexpr Ref(Ref&& rhs) noexcept(true) : rc_(rhs.rc_) { rhs.rc_ = nullptr; }

    constexpr Ref& operator=(Ref const& rhs) noexcept(true) { return *(new (this) Ref(rhs)); }
    constexpr Ref& operator=(Ref&& rhs) noexcept(true) { return *(new (this) Ref(std::move(rhs))); }

    template <typename U>
        requires(!std::is_same_v<std::remove_cv_t<U>, std::remove_cv_t<T>>) &&
                (std::is_assignable_v<std::remove_cv_t<U*&>, std::remove_cv_t<T*>>)
    operator Ref<U>() {
        Ref<U> ret;
        ret.rc_ = (detail::RefControl<U>*) (this->rc_->retain());
        return ret;
    }

    constexpr T* get(this auto self) noexcept(true) {
        if (!self.rc_) { return nullptr; }
        return (T*) &self.rc_->obj_;
    }

    constexpr T* operator->(this auto self) noexcept(true) { return NullRef::check(self.get()); }
    constexpr T& operator*(this auto self) { return *(self.operator->()); }
};
static_assert(std::semiregular<Ref<int>>);
static_assert(sizeof(Ref<int>) == sizeof(uintptr_t));

template <typename U, typename... A>
constexpr Ref<U> ref(A&&... args) {
    Ref<U> ret;
    ret.rc_ = new detail::RefControl<U>(args...);
    return ret;
}

}  // namespace cxx

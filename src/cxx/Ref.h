#pragma once
#include <concepts>
#include <cstdint>
static_assert(__cplusplus >= 202300L, "cxx-libs requires C++23");
// (c) 2024 Steve O'Brien -- MIT License

#include "Exception.h"
#include "detail/_ref.h"

#include <cassert>
#include <cstddef>
#include <type_traits>
#include <utility>

namespace cxx {

template <class T>
class Ref final : public detail::RefBase {

protected:
    detail::RefCountedBase const* obj_ {nullptr};

public:
    struct NullRef final : Exception {};

    void clear() noexcept(true) {
        if (obj_ && obj_->dec()) {
            auto* obj = obj_;
            obj_ = nullptr;
            delete obj;
        }
    }

    ~Ref() noexcept(true) { clear(); }

    Ref() = default;

    Ref(T* obj) noexcept(true) : obj_(obj) {
        assert(obj);
        // obj's `rc` is zero-initialized, which indicates
        // a reference count of 1; don't increment it.
    }

    Ref(Ref const& rhs) noexcept(true) : obj_(rhs.get()) { obj_->inc(); }

    Ref& operator=(Ref const& rhs) noexcept(true) {
        clear();
        new (this) Ref(rhs);
        return *this;
    }

    Ref(Ref&& rhs) noexcept(true) : obj_(rhs.get()) {
        // Steal this obj pointer, clear it in rhs.
        // Avoids the needs to increment / decrement refcount.
        rhs.obj_ = nullptr;
    }

    Ref& operator=(Ref&& rhs) noexcept(true) {
        // Steal this obj pointer, clear it in rhs.
        // Avoids the needs to increment / decrement refcount.
        this->obj_ = rhs.obj_;
        rhs.obj_ = nullptr;
        return *this;
    }

    template <typename U>
        requires(!std::is_same_v<std::remove_cv_t<U>, std::remove_cv_t<T>>) operator Ref<U>() {
        U* obj = (U*) obj_;
        obj_->inc();
        return Ref<U>(obj);
    }

    T* get() noexcept(true) { return (T*) obj_; }
    T const* get() const noexcept(true) { return (T const*) obj_; }

    T* operator->() noexcept(true) { return get(); }
    T const* operator->() const noexcept(true) { return get(); }

    T& operator*() {
        if (!obj_) { throw NullRef(); }
        return *obj_;
    }

    T const& operator*() const {
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

public:
    Ref<T> ref() noexcept(true) {
        auto ret = Ref((T*) (this));
        inc();
        return ret;
    }
};

template <typename U, typename... A>
static Ref<U> make(A&&... args) {
    U* ptr = new U(std::forward<A>(args)...);
    return Ref<U>(ptr);
}

}  // namespace cxx

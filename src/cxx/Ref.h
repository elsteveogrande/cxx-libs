#pragma once
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
class Ref final : private detail::RefBase {

public:
    struct NullRef final : Exception {};

    void clear() noexcept(true) {
        if (obj_ && dec()) {
            auto* obj = obj_;
            obj_ = nullptr;
            delete obj;
        }
    }

    ~Ref() noexcept(true) { clear(); }

    Ref() : detail::RefBase(nullptr) {}

    Ref(T* obj) noexcept(true) : detail::RefBase(obj) {
        assert(obj);
        // obj's `rc` is zero-initialized, which indicates
        // a reference count of 1; don't increment it.
    }

    Ref(Ref const& rhs) noexcept(true) : detail::RefBase(rhs.get()) { inc(); }

    Ref& operator=(Ref const& rhs) noexcept(true) {
        clear();
        new (this) Ref(rhs);
        return *this;
    }

    Ref(Ref&& rhs) noexcept(true) : detail::RefBase(rhs.get()) {
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
        U* obj = (U*) this->rawPointer();
        inc();
        return Ref<U>(obj);
    }

    T* get() noexcept(true) { return (T*) rawPointer(); }
    T const* get() const noexcept(true) { return (T const*) rawPointer(); }

    T* operator->() noexcept(true) { return get(); }
    T const* operator->() const noexcept(true) { return get(); }

    T& operator*() {
        if (!rawPointer()) { throw NullRef(); }
        return *rawPointer();
    }

    T const& operator*() const {
        if (!rawPointer()) { throw NullRef(); }
        return *rawPointer();
    }
};

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

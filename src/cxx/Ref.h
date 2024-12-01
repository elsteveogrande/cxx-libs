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

template <class T> class Ref final : private detail::RefBase {

public:
    struct NullRef final : Exception {};

    ~Ref() noexcept(true) {
        if (obj_ && dec()) {
            auto* obj = obj_;
            obj_ = nullptr;
            delete obj;
        }
    }

    Ref(T* obj) noexcept(true) : detail::RefBase(obj) {
        assert(obj);
        // obj's `rc` is zero-initialized, which indicates
        // a reference count of 1; don't increment it.
    }

    Ref(Ref<T> const& rhs) noexcept(true) : detail::RefBase(rhs.get()) {
        inc();
    }

    template <typename U>
    requires(!std::is_same_v<std::remove_cv_t<U>, std::remove_cv_t<T>>)
    operator Ref<U>() {
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

template <typename T> class RefCounted : public detail::RefCountedBase {
protected:
    virtual ~RefCounted() = default;

public:
    Ref<T> ref() noexcept(true) { return Ref((T*) (this)); }
};

template <typename U, typename... A> static Ref<U> make(A&&... args) {
    U* ptr = new U(std::forward<A>(args)...);
    return Ref<U>(ptr);
}

}  // namespace cxx

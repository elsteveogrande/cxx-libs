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
struct RefControl final {
    // clang-format off
    /**
     * This class would only be instantiated on a concrete (non-abstract)
     * type `T`.  No creation of `RefControl<T>` would occur when T is an
     * abstract type.  However this template will get instantiated regardless,
     * and still needs to compile properly.  The default destructor created for
     * `RefControl<Abstract>` would be invalid, so we replace the abstract type
     * with some dummy (instantiable even if trivial) type.
     */
    constexpr static bool hasObject = !std::is_abstract_v<T>;

    template <bool B> struct ObjTypeSelector { using type = int; };
    template <> struct ObjTypeSelector<true> { using type = T; };
    using TT = typename ObjTypeSelector<hasObject>::type;

    std::atomic_int64_t mutable refs_ {0};

    /** Either a concrete `T` instance, or some substitute type. */
    TT obj_;

    template <typename... A>
    RefControl(A&&... args) : obj_(TT(args...)) {}
    ~RefControl() = default;
    T* obj() { return (T*) &obj_; }
    RefControl* retain() { ++this->refs_; return this; }
    void release() { if (--this->refs_ == -1) { delete this; } }
    // clang-format on
};

}  // namespace cxx::detail

namespace cxx {

template <class T>
struct Ref final {
    detail::RefControl<T>* rc_ {nullptr};

    struct NullRef final : Exception {
        template <typename U>
        constexpr static U* check(U* ptr) {
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

    constexpr T* get(this auto& self) noexcept(true) {
        if (!self.rc_) { return nullptr; }
        return self.rc_->obj();
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

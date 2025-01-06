#pragma once
static_assert(__cplusplus >= 202300L, "cxx-libs requires C++23");
// (c) 2024 Steve O'Brien -- MIT License

#include "_Exception.h"

#include <atomic>
#include <cassert>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <type_traits>
#include <utility>

namespace cxx::detail {

template <typename T>
class Ref0;

template <typename T>
struct RefControl final {
    // clang-format off
    /** This class would only be instantiated on a concrete (non-abstract)
     * type `T`.  No creation of `RefControl<T>` would occur when T is an
     * abstract type.  However this *template* will get instantiated regardless,
     * and still needs to compile properly.  The default destructor created for
     * `RefControl<Abstract>` would be invalid, so we replace the abstract type
     * with some dummy (instantiable even if trivial) type. */
    constexpr static bool hasObject = !std::is_abstract_v<T>;
 
    template <bool B> struct ObjTypeSelector { using type = int; };
    template <> struct ObjTypeSelector<true> { using type = T; };
    using TT = typename ObjTypeSelector<hasObject>::type;

    /** Reference count, initially 0, and incremented when this RC is assigned to some `Ref` */
    std::atomic_int64_t mutable refs_ {0};

    /** Either a concrete `T` instance, or some substitute type. */
    TT obj_;

    ~RefControl() noexcept { }

    RefControl(RefControl const& rhs) = delete;
    RefControl& operator=(RefControl const& rhs) = delete;

    template <typename... A>
    RefControl(A&&... args) noexcept : obj_ {args...} {}

    T* obj() noexcept{ return reinterpret_cast<T*>(&obj_); }
    int64_t refs() const noexcept{ return refs_.load(std::memory_order_relaxed); }

    auto* retain(this auto& self) noexcept {
        ++self.refs_;
        // printf("retain: this=%p refs=%d\n", &self, self.refs());
        // StackResolver sr;
        // StackTrace st;
        // st.resolve(sr);
        // st.dump(std::cerr);
        return &self;
    }

    void release(this auto& self) noexcept {
        if (!--self.refs_) {
            delete &self;
            // printf("DELETE %p\n", &self);
        } else {
            // printf("release: this=%p refs=%d\n", &self, self.refs()); }
            // StackResolver sr;
            // StackTrace st;
            // st.resolve(sr);
            // st.dump(std::cerr);
        }
        // clang-format on
    };
};

}  // namespace cxx::detail

namespace cxx {

struct NullRef final : Exception {
    template <typename U>
    static U* check(U* ptr) {
        if (!ptr) { throw NullRef(); }
        return ptr;
    }
};

template <class T>
struct Ref final {
    detail::RefControl<T>* rc_ {nullptr};

    ~Ref() noexcept { clear(); }
    Ref() noexcept = default;

    Ref(detail::RefControl<T>* rc) noexcept : rc_(rc ? rc->retain() : nullptr) {
        // printf("Ref(&rc): this=%p rc=%p refs=%d\n", this, rc_, refs());
        // fflush(stdout);
        // fflush(stderr);
    }

    Ref<T>(Ref<T> const& rhs) noexcept : Ref(rhs.rc_) {}
    Ref<T>(Ref<T>&& rhs) noexcept : rc_(rhs.rc_) { rhs.rc_ = nullptr; }
    Ref<T>& operator=(Ref<T> const& rhs) noexcept { return *new (this) Ref(rhs); }
    Ref<T>& operator=(Ref<T>&& rhs) noexcept { return *new (this) Ref(std::move(rhs)); }

    template <typename U>
    Ref<T>(Ref<U> const& rhs) noexcept : Ref((detail::RefControl<T>*) rhs.rc_) {}

    template <typename U>
    Ref<T>(Ref<U>&& rhs) noexcept : rc_((detail::RefControl<T>*) rhs.rc_) {
        rhs.rc_ = nullptr;
    }

    template <typename U>
    Ref<T>& operator=(Ref<U> const& rhs) noexcept {
        return *new (this) Ref(rhs);
    }

    template <typename U>
    Ref<T>& operator=(Ref<U>&& rhs) noexcept {
        return *new (this) Ref(std::move(rhs));
    }

    T* get() noexcept { return rc_ ? rc_->obj() : nullptr; }
    T& operator*() noexcept { return *(operator->()); }
    T* operator->() { return NullRef::check(get()); }

    T const* get() const noexcept { return rc_ ? rc_->obj() : nullptr; }
    T const& operator*() const noexcept { return *(operator->()); }
    T const* operator->() const { return NullRef::check(get()); }

    void clear() noexcept {
        if (rc_) {
            rc_->release();
            rc_ = nullptr;
        }
    }

    operator bool() const noexcept { return !!rc_; }

    uint64_t refs() const noexcept { return rc_ ? rc_->refs() : 0; }

    template <typename... A>
    static Ref<T> make(A&&... args) noexcept {
        return Ref(new detail::RefControl<T>(std::forward<A>(args)...));
    }
};
static_assert(std::semiregular<Ref<int>>);
static_assert(sizeof(Ref<int>) == sizeof(uintptr_t));

}  // namespace cxx

#include "../Exception.h"

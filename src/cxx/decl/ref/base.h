#pragma once
#include <type_traits>
static_assert(__cplusplus >= 202300L, "cxx-libs requires C++23");
// (c) 2024 Steve O'Brien -- MIT License

#include <concepts>
#include <cstddef>
#include <cstdint>

namespace cxx {

template <typename T>
struct Ref final {
    struct Block {
        uint64_t refs_ {0};
        T* obj_ {nullptr};
    };

    Block* block_ {nullptr};

    void clear();
    Ref<T>& copyFrom(Ref<T> const& rhs);
    Ref<T>& moveFrom(Ref<T>&& rhs);

    ~Ref() { clear(); }
    Ref() = default;
    Ref(nullptr_t) : Ref() {}
    Ref(Ref<T> const& rhs) { copyFrom(rhs); }
    Ref(Ref<T>&& rhs) { moveFrom(std::move(rhs)); }
    Ref& operator=(Ref<T> const& rhs) { return copyFrom(rhs); }
    Ref& operator=(Ref<T>&& rhs) { return moveFrom(std::move(rhs)); }

    // clang-format off
    template <typename U> requires(std::is_convertible_v<T*, U*>)
    operator Ref<U>&() { return *(Ref<U>*) (this); }

    template <typename U> requires(std::is_assignable_v<U*, T*>)
    Ref(Ref<U> const& rhs) { copyFrom(rhs); }

    template <typename U> requires(std::is_assignable_v<U*, T*>)
    Ref(Ref<U>&& rhs) { moveFrom(rhs); }

    template <typename U> requires(std::is_assignable_v<U*, T*>)
    Ref& operator=(Ref<U> const& rhs) { return copyFrom(rhs); }

    template <typename U> requires(std::is_assignable_v<U*, T*>)
    Ref& operator=(Ref<U>&& rhs) { return moveFrom(rhs); }

    template <typename... A> requires (!std::is_array_v<T>)
    static Ref<T> make(A... args) {
        Ref<T> ret;
        ret.block_ = new Block();
        ret.block_->obj_ = new T(std::forward<A>(args)...);
        ret.block_->refs_ = 1;
        return ret;
    }

    template <typename... A> requires (std::is_array_v<T>)
    static Ref<T> make(size_t size) {
        using E = typename std::remove_extent_t<T>;
        Ref<E> ref;
        ref.block_ = new Ref<E>::Block();
        E* array = (E*) new E[size];
        ref.block_->obj_ = (E*) array;
        ref.block_->refs_ = 1;
        auto& ret = (Ref<T>&) ref;
        return ret;
    }

    // clang-format on

    operator bool() const { return block_ != nullptr; }
    T* get(this auto& self) { return self.block_ ? self.block_->obj_ : nullptr; }
    T* operator->(this auto& self) { return self.get(); }
    T& operator*(this auto& self) { return *self.get(); }

    uint64_t _refs() const { return block_ ? block_->refs_ : 0; }
};
static_assert(std::semiregular<Ref<int>>);

}  // namespace cxx

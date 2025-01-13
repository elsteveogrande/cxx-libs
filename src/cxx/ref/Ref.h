#pragma once
#include <concepts>
static_assert(__cplusplus >= 202300L, "cxx-libs requires C++23");
// (c) 2024 Steve O'Brien -- MIT License

#include "../Concepts.h"

#include <cstddef>
#include <cstdint>
#include <functional>
#include <type_traits>

namespace cxx {

template <typename A, typename B>
concept Compatible = std::is_base_of_v<A, B> && (!SameRCV<A, B>);

template <typename T>
struct Ref final {
    struct Block {
        uint64_t refs_ {0};
        T* obj_ {nullptr};
        std::function<void()> deleter_;
    };

    Block* block_ {nullptr};

    void clear() {
        if (!block_) { return; }
        if (!--block_->refs_) {
            block_->deleter_();
            delete block_;
        }
        block_ = nullptr;
    }

    template <typename U>
    Ref<T>& copyFrom(Ref<U> const& rhs) {
        clear();
        block_ = (Block*) rhs.block_;
        if (block_) { ++block_->refs_; }
        return *this;
    }

    template <typename U>
    Ref<T>& moveFrom(Ref<U>&& rhs) {
        clear();
        block_ = (Block*) rhs.block_;
        rhs.block_ = nullptr;
        return *this;
    }

    ~Ref() { clear(); }
    Ref() = default;
    Ref(nullptr_t) : Ref() {}
    Ref(Ref<T> const& rhs) { copyFrom<T>(rhs); }
    Ref(Ref<T>&& rhs) { moveFrom<T>(std::move(rhs)); }
    Ref& operator=(Ref<T> const& rhs) { return copyFrom<T>(rhs); }
    Ref& operator=(Ref<T>&& rhs) { return moveFrom<T>(std::move(rhs)); }

    // clang-format off
    template <typename U> requires Compatible<U, T>
    operator Ref<U>& () { return *(Ref<U>*) this;}

    template <typename U> requires Compatible<U, T>
    operator Ref<U> const& () const { return *(Ref<U>*) this;}

    template <typename... A> requires (!std::is_array_v<T>)
    static Ref<T> make(A... args) {
        Ref<T> ret;
        ret.block_ = new Block();
        auto* obj = new T(std::forward<A>(args)...);
        ret.block_->obj_ = obj;
        ret.block_->deleter_ = [obj] { delete obj; };
        ret.block_->refs_ = 1;
        return ret;
    }

    template <typename... A> requires (std::is_array_v<T>)
    static auto make(size_t size) {
        using E = typename std::remove_extent_t<T>;
        Ref<E> ref;
        ref.block_ = new Ref<E>::Block();
        E* array = (E*) new E[size];
        ref.block_->obj_ = (E*) array;
        ref.block_->deleter_ = [array] { delete[] array; };
        ref.block_->refs_ = 1;
        auto& ret = (Ref<T>&) ref;
        return ret;
    }

    // clang-format on

    operator bool() const { return block_ != nullptr; }
    T* get(this auto& self) { return self.block_ ? self.block_->obj_ : nullptr; }
    T* operator->(this auto&& self) { return self.get(); }
    T& operator*(this auto&& self) { return *self.get(); }

    uint64_t _refs() const { return block_ ? block_->refs_ : 0; }
};
static_assert(std::semiregular<Ref<int>>);

}  // namespace cxx

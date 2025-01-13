#pragma once
#include <atomic>
static_assert(__cplusplus >= 202300L, "cxx-libs requires C++23");
// (c) 2024 Steve O'Brien -- MIT License

#include "../Concepts.h"

#include <concepts>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <type_traits>

namespace cxx {

struct NullRef;

template <typename A, typename B>
concept Compatible = std::is_base_of_v<A, B> && (!SameRCV<A, B>);

/**
 * A thread-safe shared pointer.
 * Can be empty (iff `.get()` returns `nullptr`).
 * The `operator*` will throw if this is empty.
 */
template <typename T>
struct Ref final {
    struct Block {  // impl detail: this contains the object + its reference count
        std::atomic_uint64_t refs_ {0};  // ref count.
        T* obj_ {nullptr};               // the object.  TODO: allocate T here directly
        std::function<void()> deleter_;  // need a delete function, since we do some type-puns
    };

    Block* block_ {nullptr};  // empty by default

    auto* check(this auto self);

    void clear() {
        if (!block_) { return; }  // already empty, nothing to do
        if (!--block_->refs_) {   // otherwise, dec refcount, and no more refs, ...
            block_->deleter_();   // delete the object within the block
            delete block_;        // then delete the block
        }
        block_ = nullptr;  // clear pointer so we can't refer to object anymore
    }

    template <typename U>
    Ref<T>& copyFrom(Ref<U> const& rhs) {
        clear();                          // first release our reference (if any)
        block_ = (Block*) rhs.block_;     // copy other's block
        if (block_) { ++block_->refs_; }  // bump reference count
        return *this;
    }

    template <typename U>
    Ref<T>& moveFrom(Ref<U>&& rhs) {
        clear();                       // first release our reference (if any)
        block_ = (Block*) rhs.block_;  // take other's block; no change in refcount
        rhs.block_ = nullptr;          // we took their block, so clear their pointer
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
        ret.block_ = new Block();                       // alloc a new Block
        auto* obj = new T(std::forward<A>(args)...);    // construct object w/ given args
        ret.block_->obj_ = obj;                         // we're now managing a pointer to that obj
        ret.block_->deleter_ = [obj] { delete obj; };   // provide deletion func
        ret.block_->refs_ = 1;                          // initially only one `Ref` exists
        return ret;
    }

    template <typename... A> requires (std::is_array_v<T>)
    static auto make(size_t size) {
        using E = typename std::remove_extent_t<T>;     // the element type of the array of type T
        Ref<E> ref;                         // we'll actually construct a ref<E> and type-pun below
        ref.block_ = new Ref<E>::Block();   // new block with an E* pointer (points to first element)
        E* array = (E*) new E[size];        // allocate the array
        ref.block_->obj_ = (E*) array;      // which we'll manage via this block
        ref.block_->deleter_ = [array] { delete[] array; };     // deletion func
        ref.block_->refs_ = 1;              // initially only one `Ref` exists
        auto& ret = (Ref<T>&) ref;          // return as the expected type
        return ret;
    }

    // clang-format on

    operator bool() const { return block_ != nullptr; }
    T* get(this auto& self) { return self.block_ ? self.block_->obj_ : nullptr; }
    T* operator->(this auto&& self) { return self.get(); }
    T& operator*(this auto&& self) { return *self.check(); }

    uint64_t _refs() const {
        return block_ ? block_->refs_.load(std::memory_order_relaxed) : uint64_t(0);
    }
};
static_assert(std::semiregular<Ref<int>>);

}  // namespace cxx

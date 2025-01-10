#pragma once
static_assert(__cplusplus >= 202300L, "cxx-libs requires C++23");
// (c) 2024 Steve O'Brien -- MIT License

namespace cxx {
template <typename T>
struct Ref;
}

#include "decl/ref/base.h"

namespace cxx {

template <typename T>
void Ref<T>::clear() {
    if (block_) {
        if (!--block_->refs_) {
            delete block_->obj_;
            delete block_;
        }
        block_ = nullptr;
    }
}

template <typename T>
Ref<T>& Ref<T>::copyFrom(Ref<T> const& rhs) {
    clear();
    block_ = rhs.block_;
    if (block_) { ++block_->refs_; }
    return *this;
}

template <typename T>
Ref<T>& Ref<T>::moveFrom(Ref<T>&& rhs) {
    clear();
    block_ = rhs.block_;
    rhs.block_ = nullptr;
    return *this;
}

}  // namespace cxx

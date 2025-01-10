#pragma once
static_assert(__cplusplus >= 202300L, "cxx-libs requires C++23");
// (c) 2024 Steve O'Brien -- MIT License

#include "base.h"

#include <utility>

namespace cxx {

void String::destroy() {
    // If this is SHARED then we need to clear the contained Ref; otherwise no-op.
    if (type() == Type::SHARED) { asShared().~Shared(); }
}

void String::copyFrom(cxx::String const& rhs) {
    this->size_ = rhs.size_;
    switch (rhs.type()) {
    case SHARED:
        this->asShared() = rhs.asShared();  // invoke Ref copy-assignment
        break;
    default: this->data_ = rhs.data_;  // simple 8-byte copy of pointer or characters
    }
}

void String::moveFrom(cxx::String&& rhs) {
    this->size_ = rhs.size_;
    switch (rhs.type()) {
    case SHARED:
        this->asShared() = std::move(rhs.asShared());  // invoke Ref move-assignment
        rhs.size_ = 0;                                 // clear size, since rhs is moved-out-of
        break;
    default: this->data_ = rhs.data_;  // simple 8-byte copy of pointer or characters
    }
}

}  // namespace cxx

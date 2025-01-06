#pragma once
static_assert(__cplusplus >= 202300L, "cxx-libs requires C++23");
// (c) 2024 Steve O'Brien -- MIT License

#include "decl/_Bytes.h"

namespace cxx {

Cursor ByteBuffer::cur() const {
    BytesSP self = shared_from_this();
    return {shared_from_this(), data_, size_};
}

}  // namespace cxx

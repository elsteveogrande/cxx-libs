#pragma once
static_assert(__cplusplus >= 202300L, "cxx-libs requires C++23");
// (c) 2024 Steve O'Brien -- MIT License

#include "decl/_Bytes.h"
#include "decl/_Cursor.h"

namespace cxx {

Cursor ByteBuffer::cur() const { return {this, data_, size_}; }

}  // namespace cxx

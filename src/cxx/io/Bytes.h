#pragma once
static_assert(__cplusplus >= 202300L, "cxx-libs requires C++23");
// (c) 2024 Steve O'Brien -- MIT License

#include <cstddef>

namespace cxx {

struct Bytes;
struct Cursor;
struct File;

/** A contiguous range of `uint8_t`s in memory. */
struct Bytes {
    virtual ~Bytes() = default;
    virtual Cursor cur() const = 0;
};

}  // namespace cxx

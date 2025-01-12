#pragma once
static_assert(__cplusplus >= 202300L, "cxx-libs requires C++23");
// (c) 2024 Steve O'Brien -- MIT License

#include <cstddef>
#include <cstdint>

namespace cxx {

struct Bytes;
struct Cursor;
struct File;

/** A contiguous range of `uint8_t`s in memory. */
struct Bytes {
    virtual ~Bytes() = default;
    virtual Cursor cur() const = 0;
};

struct ByteBuffer final : Bytes {
    uint8_t const* const data_;
    size_t const size_;
    ByteBuffer(size_t size) : data_(new uint8_t[size]), size_(size) {}
    Cursor cur() const override;
};

}  // namespace cxx

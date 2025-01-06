#pragma once
static_assert(__cplusplus >= 202300L, "cxx-libs requires C++23");
// (c) 2024 Steve O'Brien -- MIT License

#include <cerrno>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cxxabi.h>
#include <dlfcn.h>
#include <execinfo.h>
#include <fcntl.h>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <sys/fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

namespace cxx {

struct Bytes;
using BytesSP = std::shared_ptr<Bytes>;
struct Cursor;
struct File;

/** A contiguous range of `uint8_t`s in memory. */
struct Bytes {
    virtual ~Bytes() = default;
    virtual uint8_t const* data() const = 0;
    virtual size_t size() const = 0;
};

struct ByteBuffer : Bytes {
    uint8_t const* const data_;
    size_t const size_;

    uint8_t const* data() const override { return data_; }
    size_t size() const override { return size_; }

    ByteBuffer(size_t size) : data_(new uint8_t[size]), size_(size) {}
};

/** A read-only reader for a `Bytes` object, providing (some) binary stream-like functions. */
struct Cursor {
    // clang-format off
    Bytes& bytes_;
    size_t const base_;
    size_t const size_;
    size_t offset_ {0};

    Cursor(Bytes& bytes, size_t base, size_t size)
            : bytes_(bytes), base_(base), size_(size) {}

    Cursor(Bytes& bytes, size_t base)
            : bytes_(bytes), base_(base), size_(bytes.size() - base) {}

    Cursor(Bytes& bytes)
            : bytes_(bytes), base_(0), size_(bytes.size()) {}

    auto peekU8(size_t adj = 0) const { return *(bytes_.data() + base_ + offset_ + adj); }
    auto peekU16(size_t adj = 0) const { return uint16_t(peekU8(adj + 1)) | (uint16_t(peekU8(adj)) << 8); }
    auto peekU32(size_t adj = 0) const { return uint32_t(peekU16(adj + 2)) | (uint32_t(peekU8(adj)) << 8); }
    auto peekU64(size_t adj = 0) const { return uint32_t(peekU32(adj + 4)) | (uint32_t(peekU32(adj)) << 8); }

    auto peekI8(size_t adj = 0) const { return int8_t(peekU8(adj)); }
    auto peekI16(size_t adj = 0) const { return int16_t(peekU16(adj)); }
    auto peekI32(size_t adj = 0) const { return int32_t(peekU32(adj)); }
    auto peekI64(size_t adj = 0) const { return int64_t(peekU64(adj)); }

    auto u8() { auto ret = peekU8(); offset_ += 1; return ret; }
    auto u16() { auto ret = peekU16(); offset_ += 2; return ret; }
    auto u32() { auto ret = peekU32(); offset_ += 4; return ret; }
    auto u64() { auto ret = peekU64(); offset_ += 8; return ret; }

    auto i8() { return (int8_t) u8(); }
    auto i16() { return (int16_t) u16(); }
    auto i32() { return (int32_t) u32(); }
    auto i64() { return (int64_t) u64(); }

    auto _expect(auto actual, auto expect, bool eq, std::string const& err) {
        if ((actual == expect) ^ eq) { throw std::runtime_error(err); }
        return actual;
    }

    auto u8(auto expect, bool eq, auto err) { return _expect(u8(), expect, eq, err); }
    auto u16(auto expect, bool eq, auto err) { return _expect(u16(), expect, eq, err); }
    auto u32(auto expect, bool eq, auto err) { return _expect(u32(), expect, eq, err); }
    auto u64(auto expect, bool eq, auto err) { return _expect(u64(), expect, eq, err); }

    auto i8(auto v, bool eq, auto err) { return (int8_t) i8(v, eq, err); }
    auto i16(auto v, bool eq, auto err) { return (int16_t) i16(v, eq, err); }
    auto i32(auto v, bool eq, auto err) { return (int32_t) i32(v, eq, err); }
    auto i64(auto v, bool eq, auto err) { return (int64_t) i64(v, eq, err); }

    std::string str() {
        std::stringstream ret;
        while (true) {
            char c = u8();
            if (!c) { break; }
            ret << c;
        }
        return ret.str();
    }

    std::string fixedStr(unsigned size) {
        std::stringstream ret;
        while (size--) {
            char c = u8();
            if (c) { ret << c; }
        }
        return ret.str();
    }

    uint64_t uleb() {
        uint64_t ret = 0;
        int r = 64;
        auto roll = [&]() {
            ret = (ret >> 7) | (ret << 57);
            r -= 7;
        };
        while (true) {
            auto b = u8();
            ret |= (uint64_t) (b & 0x7f);
            roll();
            if (!(b >> 7)) { break; }
        }
        r %= 64;
        return ret >> r;
    }

    void operator=(size_t offset) { offset_ = offset; }
    void operator+=(size_t skip) { offset_ += skip; }

    // clang-format on
};

}  // namespace cxx

#pragma once
static_assert(__cplusplus >= 202300L, "cxx-libs requires C++23");
// (c) 2024 Steve O'Brien -- MIT License

#include "_Bytes.h"

#include <cstddef>
#include <cstdint>
#include <sstream>
#include <stdexcept>
#include <string>

namespace cxx {

/** A read-only reader for a `Bytes` object, providing (some) binary stream-like functions. */
struct Cursor final {
    // clang-format off
    Bytes const* owner_;            // owns the actual bytes
    uint8_t const* const base_ {};  // start of accessible window
    size_t const size_ {};          // max size of this cursor's window; last byte is at (base + size - 1)
    size_t offset_ {0};             // initially zero, adjusted during reads

    ~Cursor() noexcept = default;

    Cursor(Bytes const* owner, uint8_t const* base, size_t size)
        : owner_(owner), base_(base), size_(size) {}

    Cursor(Cursor const& rhs)
        : owner_(rhs.owner_), base_(rhs.base_ + rhs.offset_), size_(rhs.size_ - rhs.offset_) {}

    Cursor& operator=(Cursor const& rhs) { return *new (this) Cursor(rhs); }

    bool operator==(Cursor const& rhs) const { return (base_ + offset_) == (rhs.base_ + rhs.offset_); }
    bool operator<(Cursor const& rhs) const { return (base_ + offset_) < (rhs.base_ + rhs.offset_); }

    uint8_t peekU8(size_t adj = 0) const { return *(base_ + offset_ + adj); }
    uint16_t peekU16(size_t adj = 0) const { return uint16_t(peekU8(adj + 1)) << 8 | (uint16_t(peekU8(adj))); }
    uint32_t peekU32(size_t adj = 0) const { return uint32_t(peekU16(adj + 2)) << 16 | (uint32_t(peekU16(adj))); }
    uint64_t peekU64(size_t adj = 0) const { return uint64_t(peekU32(adj + 4)) << 32 | (uint64_t(peekU32(adj))); }

    int8_t peekI8(size_t adj = 0) const { return int8_t(peekU8(adj)); }
    int16_t peekI16(size_t adj = 0) const { return int16_t(peekU16(adj)); }
    int32_t peekI32(size_t adj = 0) const { return int32_t(peekU32(adj)); }
    int64_t peekI64(size_t adj = 0) const { return int64_t(peekU64(adj)); }

    uint8_t u8() { auto ret = peekU8(); offset_ += 1; return ret; }
    uint16_t u16() { auto ret = peekU16(); offset_ += 2; return ret; }
    uint32_t u32() { auto ret = peekU32(); offset_ += 4; return ret; }
    uint64_t u64() { auto ret = peekU64(); offset_ += 8; return ret; }

    int8_t i8() { return (int8_t) u8(); }
    int16_t i16() { return (int16_t) u16(); }
    int32_t i32() { return (int32_t) u32(); }
    int64_t i64() { return (int64_t) u64(); }

    auto _expect(auto actual, auto expect, bool eq, std::string const& err) {
        if ((actual == expect) ^ eq) { throw std::runtime_error(err); }
        return actual;
    }

    auto u8(auto expect, bool eq, auto err) { return _expect(u8(), expect, eq, err); }
    auto u16(auto expect, bool eq, auto err) { return _expect(u16(), expect, eq, err); }
    auto u32(auto expect, bool eq, auto err) { return _expect(u32(), expect, eq, err); }
    auto u64(auto expect, bool eq, auto err) { return _expect(u64(), expect, eq, err); }

    auto i8(auto v, bool eq, auto err) { return (int8_t) u8(v, eq, err); }
    auto i16(auto v, bool eq, auto err) { return (int16_t) u16(v, eq, err); }
    auto i32(auto v, bool eq, auto err) { return (int32_t) u32(v, eq, err); }
    auto i64(auto v, bool eq, auto err) { return (int64_t) u64(v, eq, err); }

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
        bool terminated = false;
        while (size--) {
            char c = u8();
            if (c == 0) { terminated = true; }
            if (!terminated) { ret << c; }
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

    uint64_t sleb() {
        uint64_t ret = 0;
        int r = 64;
        while (true) {
            uint64_t b = u8();
            ret >>= 7;
            ret |= (b << 57);
            r -= 7;
            if (!(b & 0x80)) { break; }
        }
        return int64_t(ret) >> r;
    }

    void operator=(size_t offset) { offset_ = offset; }
    void operator+=(size_t skip) { offset_ += skip; }

    Cursor operator+(size_t adj) const { return {owner_, base_ + adj, size_ - adj}; }

    // clang-format on
};

}  // namespace cxx

#pragma once
static_assert(__cplusplus >= 202300L, "cxx-libs requires C++23");
// (c) 2024 Steve O'Brien -- MIT License

#include "Generator.h"
#include "Ref.h"
#include "Util.h"

#include <cassert>
#include <concepts>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <string>
#include <utility>

namespace cxx {
namespace detail {

constexpr static char const* kEmpty = "";
constexpr static size_t kSmallMax = 15;

/**
 * Array of characters created with `new`; note that this struct only houses the first
 * char, and is over-allocated, so the rest of the string goes beyond this structure.
 */
struct HeapString final : RefCounted<HeapString> {
    /** First byte of character array (the rest of the allocated bytes are just past this one) */
    char data_ {0};

    static HeapString* make(char const* src, size_t size) {
        auto bytes = sizeof(HeapString) + size;  // accomodates HeapString + chars + NUL
        char* data = new char[bytes];            // home of our new HeapString
        auto* ret = new (data) HeapString();     // "bless" this new HeapString instance
        ce_memcpy(&ret->data_, src, size + 1);   // copy source string including its NUL
        return ret;
    }
};

struct SmallString final {
    char data_[16] {0};

    constexpr SmallString() = default;
    constexpr SmallString(char const* src, size_t offset, size_t size) {
        auto* const s = src + offset;
        auto* const d = data_;
        size_t i = 0;
        for (; i < size; i++) { d[i] = s[i]; }
        for (; i < size; i++) { data_[i] = 0; }
    }
};
static_assert(sizeof(SmallString) <= 16);
static_assert(std::semiregular<SmallString>);

struct RegString final {
    char const* data_ {kEmpty};
    Ref<HeapString> heapRef_ {};  // empty IFF data_ is a literal string
};
static_assert(sizeof(RegString) <= 16);
static_assert(std::semiregular<RegString>);

}  // namespace detail

class String final {
    size_t size_;
    union Storage {
        detail::SmallString small_;
        // RegString regular_;
    } storage_;

    consteval static Storage buildStorageCEV(char const* src, size_t offset, size_t size) {
        if (size <= detail::kSmallMax) { return Storage {.small_ = {src, offset, size}}; }
        std::unreachable();
    }

    constexpr static Storage buildStorage(char const* src, size_t offset, size_t size) {
        if consteval { return buildStorageCEV(src, offset, size); }
        if (size <= detail::kSmallMax) { return Storage {.small_ = {src, offset, size}}; }
        std::unreachable();
    }

public:
    constexpr ~String() {
        if consteval { return; }
    }

    constexpr String(char const* src, size_t offset, size_t size)
            : size_(size)
            , storage_(buildStorage(src, offset, size)) {}

    constexpr String() : String(detail::kEmpty, 0, 0) {}

    constexpr size_t size() const { return size_; }

    constexpr char const* data() const {
        if (size_ <= detail::kSmallMax) { return storage_.small_.data_; }
        std::unreachable();
    }

    constexpr String(std::string const& s) : String(s.data(), 0, s.size()) {}
    constexpr String(char const* src) : String(src, 0, ce_strlen(src)) {}

    constexpr operator char const*() const { return data(); }
    constexpr char operator[](size_t pos) const { return *(data() + pos); }

    constexpr bool operator==(String const& rhs) const {
        if (this == &rhs) { return true; }
        if (size() != rhs.size()) { return false; }
        return (*this) == rhs.data();  // delegate to `operator==(char const*)`
    }

    constexpr bool operator<(char const* rhs) const { return compare(rhs) < 0; }
    constexpr bool operator==(char const* rhs) const { return !compare(rhs); }

    constexpr int compare(char const* rhs) const {
        char const* a = data();
        char const* b = rhs;
        int ret = 0;
        do { ret = (*b - *a); } while (ret == 0 && *a++ && *b++);
        return ret;
    }

    constexpr String operator+(String const& rhs) const {
        auto total = size() + rhs.size();
        char chars[total + 1];
        ce_memcpy(chars, data(), size());
        ce_memcpy(chars + size(), rhs.data(), rhs.size());
        chars[total] = 0;
        return {chars, 0, total};
    }

    // `String` is not yet complete, so use a placeholder S.
    // This will be instantiated out-of-line below.
    template <typename S = String>
    Generator<S> split(char sep);
};
static_assert(std::regular<String>);
static_assert(sizeof(String) == 24);

template <>
Generator<String> String::split(char sep) {
    size_t const size = this->size();
    char const* data = this->data();
    size_t start = 0;  // current [initial] part starts here
    size_t pos = 0;    // current end position (exclusive)
    for (; pos < size; ++pos) {
        if (data[pos] == sep) {
            co_yield String(data, start, pos - start);
            start = pos + 1;  // skip past this char
        }
    }
    co_yield String(data + start, start, pos - start);
}

}  // namespace cxx

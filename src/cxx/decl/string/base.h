#pragma once
static_assert(__cplusplus >= 202300L, "cxx-libs requires C++23");
// (c) 2024 Steve O'Brien -- MIT License

#include "../_Generator.h"
#include "../ref/base.h"

#include <cassert>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <string>
#include <utility>

namespace cxx {

namespace detail {

constexpr size_t ceStringLength(char const* data) {
    size_t i = 0;
    while (data[i]) { ++i; }
    return i;
}

constexpr void ceStringCopy(char* dest, char const* src, size_t size, size_t bufLimit) {
    size_t i = 0;
    for (; i < size; i++) { dest[i] = src[i]; }
    for (; i < bufLimit; i++) { dest[i] = 0; }
}

}  // namespace detail

class String final {
    constexpr static size_t kSmallMax = sizeof(void*);
    constexpr static char const* kEmpty = "";

    int64_t size_ {0};
    char const* data_ {nullptr};

    enum Type { LITERAL, SMALL, SHARED };

    constexpr Type type() const {
        if consteval { return LITERAL; }
        if (size_ < 0) { return LITERAL; }
        if (size_t(size_) < kSmallMax) { return SMALL; }
        return SHARED;
    }

    struct Small final {
        char chars_[8];
    };

    struct Shared final {
        Ref<char[]> chars_;
    };

    // Type-pun support
    Small& asSmall() const { return *(Small*) &data_; }
    Shared& asShared() const { return *(Shared*) &data_; }
    char const* asLiteral() const { return data_ ? data_ : kEmpty; }

    // Defined in ctor.h:

    void destroy();
    void copyFrom(String const& rhs);
    void moveFrom(String&& rhs);

public:
    // Empty strings are trivially-(zero-)constructible.
    constexpr String() noexcept = default;

    constexpr ~String() noexcept {
        if consteval { return; }
        destroy();
    }

    constexpr String(String const& rhs) noexcept {
        if consteval {
            size_ = rhs.size_;
            data_ = rhs.data_;
            return;
        }
        copyFrom(rhs);
    }

    constexpr String(String&& rhs) noexcept {
        if consteval {
            size_ = rhs.size_;
            data_ = std::move(rhs.data_);
            return;
        }
        moveFrom(std::move(rhs));
    }

    constexpr String& operator=(String const& rhs) noexcept {
        return (this == &rhs) ? *this : *new (this) String(rhs);
    }

    constexpr String& operator=(String&& rhs) noexcept {
        return (this == &rhs) ? *this : *new (this) String(std::move(rhs));
    }

    constexpr String(char const* cstr) noexcept : String(cstr, detail::ceStringLength(cstr)) {}
    constexpr String(char const* cstr, size_t size) noexcept : String(cstr, 0, size) {}

    constexpr String(char const* cstr, size_t offset, size_t size) noexcept {
        if consteval {
            size_ = -int64_t(size);
            data_ = cstr + offset;
            return;
        }
        size_ = int64_t(size);
        if (size < kSmallMax) {
            detail::ceStringCopy((char*) &data_, cstr + offset, size, sizeof(data_));
        } else {
            auto ref = Ref<char[]>::make(size + 1);
            detail::ceStringCopy((char*) ref.get(), cstr + offset, size, size + 1);
            this->asShared().chars_ = std::move(ref);
        }
    }

    constexpr size_t size() const { return size_ < 0 ? -size_ : size_; }

    constexpr char const* data() const {
        if consteval { return asLiteral(); }
        switch (type()) {
        case LITERAL: return asLiteral();
        case SMALL:   return asSmall().chars_;
        case SHARED:  return (char const*) asShared().chars_.get();
        }
        std::unreachable();
    }

    constexpr operator char const*() const { return data(); }
    constexpr operator std::string const() const { return {data(), size()}; }

    String(std::string const& str, /* disambig ctors */ int = 0) : String(str.data(), str.size()) {}

    constexpr char operator[](size_t index) {
        assert(index < size());
        return data()[index];
    }

    constexpr char const* begin() const { return data(); }
    constexpr char const* end() const { return data() + size(); }

    // Defined in compare.h:
    constexpr bool operator<(String const& rhs) const;
    constexpr bool operator<(char const* rhs) const;
    constexpr bool operator==(String const& rhs) const;
    constexpr bool operator==(char const* rhs) const;

    // Defined in concat.h:
    String operator+(String const& rhs) const;

    // Defined in split.h:
    Generator<String> split(char c = ' ') const;
};
static_assert(std::regular<String>);
static_assert(sizeof(String) == 16);

}  // namespace cxx

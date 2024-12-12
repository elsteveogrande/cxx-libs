#pragma once
static_assert(__cplusplus >= 202300L, "cxx-libs requires C++23");
// (c) 2024 Steve O'Brien -- MIT License

#include "Generator.h"
#include "Util.h"

#include <atomic>
#include <cassert>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <string>
#include <utility>

namespace cxx {

struct String;

struct StringBase {
    enum class Type { SMALL, LITERAL, SHARED };
    constexpr static char const* kEmpty = "";

    virtual ~StringBase() = default;

    virtual Type _type() const = 0;
    virtual size_t size() const = 0;
    virtual char const* data() const = 0;

    constexpr operator char const*() const { return data(); }
    constexpr char operator[](size_t pos) const { return *(data() + pos); }

    constexpr bool operator==(StringBase const& rhs) const {
        if (this == &rhs) { return true; }
        if (size() != rhs.size()) { return false; }
        return (*this) == rhs.data();
    }

    constexpr bool operator<(char const* rhs) const { return compare(rhs) < 0; }
    constexpr bool operator==(char const* rhs) const { return !compare(rhs); }

    constexpr int compare(char const* rhs) const {
        char const* a = data();
        char const* b = rhs;
        int ret = 0;
        do {
            ret = (*b - *a);
        } while (ret == 0 && *a++ && *b++);
        return ret;
    }
};

struct HeapString {
    std::atomic_int64_t rc_ {0};
    char data_ {0};

    static HeapString* make(size_t size) {
        auto bytes = sizeof(HeapString) + size;
        char* data = new char[bytes];
        return new (data) HeapString();
    }

    static HeapString* make(char const* src, size_t size) {
        auto bytes = sizeof(HeapString) + size;
        char* data = new char[bytes];
        auto* ret = new (data) HeapString();
        ce_memcpy(data + 8, src, size);
        *(data + 8 + size) = 0;
        return ret;
    }
};

struct String final : StringBase {
    Type type_;
    size_t size_ {0};

    // TODO!  Union all these instead of wasting storage
    HeapString* storage_ {nullptr};
    char const* literal_ {nullptr};
    char small_[8] {0, 0, 0, 0, 0, 0, 0, 0};

    constexpr Type _type() const override { return type_; }

    constexpr size_t size() const override { return size_; }

    constexpr char const* data() const override {
        if consteval {
            switch (type_) {
            case StringBase::Type::SMALL:   return small_;
            case StringBase::Type::LITERAL: return literal_;
            case StringBase::Type::SHARED:  std::unreachable();
            }
        }
        switch (type_) {
        case StringBase::Type::SMALL:   return small_;
        case StringBase::Type::LITERAL: return literal_;
        case StringBase::Type::SHARED:  return &storage_->data_;
        }
    }

    constexpr void _clear() {
        type_ = Type::SMALL;
        size_ = 0;
        small_[0] = 0;
        if (storage_) {
            if (-1 == --storage_->rc_) { delete[] storage_; }
            storage_ = nullptr;
        }
    }

    constexpr ~String() {
        if consteval { return; }
        _clear();
    }

    constexpr String()
            : String(kEmpty) {}

    constexpr String(char const* src)
            : String(src, ce_strlen(src)) {}

    constexpr String(std::string const& s)
            : String(s.data(), s.size()) {}

    constexpr String(String const& rhs) {
        if consteval {
            type_ = rhs.type_;
            size_ = rhs.size_;
            literal_ = rhs.literal_;
            storage_ = nullptr;
            cev_memcpy(small_, rhs.small_, 7);
            small_[7] = 0;
            return;
        }
        type_ = rhs.type_;
        size_ = rhs.size_;
        literal_ = rhs.literal_;
        if (rhs.storage_) {
            ++rhs.storage_->rc_;
            storage_ = rhs.storage_;
        } else {
            storage_ = nullptr;
        }
        ce_memcpy(small_, rhs.small_, 7);
        small_[7] = 0;
    }

    constexpr String& operator=(String const& rhs) {
        _clear();
        new (this) String(rhs);
        return *this;
    }

    // TODO moves can be optimized to avoid atomic inc / dec of refcount

    constexpr String(char const* src, size_t size) {
        size_ = size;
        if consteval {
            if (size < 8) {
                type_ = Type::SMALL;
                cev_memcpy(small_, src, size);
            } else {
                type_ = Type::LITERAL;
                literal_ = src;
            }
            return;
        }
        type_ = Type::SHARED;
        storage_ = HeapString::make(src, size);
    }

    constexpr String operator+(String const& rhs) const {
        auto total = size() + rhs.size();
        char chars[total + 1];
        ce_memcpy(chars, data(), size());
        ce_memcpy(chars + size(), rhs.data(), rhs.size());
        chars[total] = 0;
        return {chars, total};
    }

    Generator<String> split(char sep) const {
        // TODO share a backing string and yield `SubString`s or something
        size_t const size = this->size();
        char const* data = this->data();
        size_t start = 0;  // current [initial] part starts here
        size_t pos = 0;    // current end position (exclusive)
        while (pos < size) {
            if (data[pos] == sep) {
                co_yield String(data + start, pos - start);
                start = pos + 1;  // skip past this char
            }
            ++pos;
        }
        co_yield String(data + start, pos - start);
    }
};

}  // namespace cxx

#pragma once
static_assert(__cplusplus >= 202300L, "cxx-libs requires C++23");
// (c) 2024 Steve O'Brien -- MIT License

#include "Generator.h"

#include <atomic>
#include <cassert>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <string>
#include <type_traits>
#include <utility>

namespace cxx::detail {

constexpr static char const* kEmpty = "";
constexpr static size_t kSmallMax = 15;

constexpr size_t cstrlen(char const* data) { return std::string::traits_type::length(data); }

constexpr void cmemcpy(char* dest, char const* src, size_t size) {
    for (size_t i = 0; i < size; i++) { dest[i] = src[i]; }
}

constexpr void cmemzero(char* dest, size_t size) {
    for (size_t i = 0; i < size; i++) { dest[i] = 0; }
}

/**
 * Array of characters created with `new`; note that this struct only houses the first
 * char, and is over-allocated, so the rest of the string goes beyond this structure.
 */
struct HeapString final {
    // Note: not `cxx::RefCounted`, since that class is not constexpr-compatible,
    // and inclusion of this class in a union breaks String's constexpr-bility
    // (even when it's not used).  So we have to track refcounts here.

    /** Reference count minus 1; when this drop below 0, delete this `HeapString`. */
    std::atomic_int64_t rc_;

    /** First byte of character array (the rest of the allocated bytes are just past this one) */
    char data_;

    static HeapString* make(char const* src, size_t size) {
        auto bytes = sizeof(HeapString) + size;  // accomodates this struct + chars + NUL
        char* data = new char[bytes];            // home of our new HeapString
        auto* ret = (HeapString*) (data);        // ugly cast
        ret->rc_ = 0;                            // initial refcount is 1 (repr. as zero)
        cmemcpy(&(ret->data_), src, size + 1);   // copy source string including its NUL
        return ret;
    }

    void retain() { ++rc_; }
    void release() {
        if (--rc_ == -1) { delete this; }
    }
};

struct Small final {
    char data_[detail::kSmallMax + 1] {};

    constexpr Small() = default;

    constexpr Small(char const* src, size_t offset, size_t size) {
        auto* const s = src + offset;
        auto* const d = data_;
        size_t i = 0;
        for (; i < size; i++) { d[i] = s[i]; }
        for (; i < size; i++) { d[i] = 0; }
    }

    char const* data() const { return data_; }

    constexpr void clear() { cmemzero(data_, sizeof(data_)); }
};

struct Regular final {
    // IMPORTANT!  This class cannot have non-trivial dtor / copy / move functions
    // because we want to be able to include this in String's `Storage` union.
    // Even if this isn't used in a consteval context, the compiler will still complain.
    // `String`'s destructor (if not in consteval context) has to trigger `release()`.

    /**
     * Pointer to first char represented here.  Can differ from HeapString's data_ pointer
     * if e.g. this is a substring.
     */
    char const* data_ {nullptr};

    /** Unfortunately can't use a `cxx::Ref` because it's not constexpr. */
    detail::HeapString* ptr_ {nullptr};

    char const* data() const { return data_; }

    constexpr void clear() {
        data_ = nullptr;
        if consteval { return; }
        auto* hs = ptr_;
        ptr_ = nullptr;
        if (hs) { hs->release(); }
    }
};

}  // namespace cxx::detail

namespace cxx {

template <typename C>
concept Character = std::is_same_v<char, std::remove_cv_t<C>>;

class String final {

    uint64_t size_;

    constexpr bool isSmall() const { return size_ <= detail::kSmallMax; }
    constexpr bool isRegular() const { return !isSmall(); }

    union Storage {
        detail::Small small_ {};
        detail::Regular reg_;
        constexpr Storage() = default;
        constexpr Storage(detail::Small x) : small_(std::move(x)) {}
        constexpr Storage(detail::Regular x) : reg_(std::move(x)) {}
    } storage_;

    constexpr static Storage buildStorage(char const* src, size_t offset, size_t size) {
        if (size <= detail::kSmallMax) { return {detail::Small {src, offset, size}}; }
        if consteval { return {detail::Regular(src + offset, nullptr)}; }
        auto* ptr = detail::HeapString::make(src + offset, size);
        return {detail::Regular(&ptr->data_, ptr)};
    }

    // clang-format off

    consteval void clearCEV() {
        if (isSmall()) { storage_.small_.clear(); } else { storage_.reg_.clear(); }
        size_ = 0;
    }

    constexpr void clear() {
        if consteval { return clearCEV(); }
        if (isSmall()) { storage_.small_.clear(); } else { storage_.reg_.clear(); }
        new (this) String();
    }

    consteval String& copyFromCEV(String const& rhs) {
        size_ = rhs.size_;
        storage_ = rhs.storage_;
        return *this;
    }

    constexpr String& copyFrom(String const& rhs) {
        if consteval { return copyFromCEV(rhs); }
        if (rhs.isRegular() && rhs.storage_.reg_.ptr_) { rhs.storage_.reg_.ptr_->retain(); }
        size_ = rhs.size_;
        storage_ = rhs.storage_;
        return *this;
    }

    consteval String& moveFromCEV(String&& rhs) {
        size_ = rhs.size_;
        storage_ = rhs.storage_;
        rhs.clearCEV();
        return *this;
    }

    constexpr String& moveFrom(String&& rhs) {
        if consteval { return moveFromCEV(std::move(rhs)); }
        size_ = rhs.size_;
        storage_ = rhs.storage_;
        if (rhs.isRegular() && rhs.storage_.reg_.ptr_) { rhs.storage_.reg_.ptr_ = nullptr; }
        rhs.clear();
        return *this;
    }

    // clang-format on

public:
    constexpr ~String() { clear(); }

    /** Construct empty small string.  NB: this constructor zero-initializes this `String`. */
    constexpr String() : String(detail::kEmpty, 0, 0) {}

    constexpr String(String const& rhs) { copyFrom(rhs); }
    constexpr String(String&& rhs) { moveFrom(std::move(rhs)); }
    constexpr String& operator=(String const& rhs) { return copyFrom(rhs); }
    constexpr String& operator=(String&& rhs) { return moveFrom(std::move(rhs)); }

    template <Character C>
    constexpr String(C c) : String() {
        storage_.small_.data_[0] = c;
        storage_.small_.data_[1] = 0;
        size_ = 1;
    }

    constexpr String(char const* src, size_t offset, size_t size)
            : size_(size)
            , storage_(buildStorage(src, offset, size)) {}

    constexpr size_t size() const { return size_; }

    constexpr char const* data() const {
        return isSmall() ? storage_.small_.data() : storage_.reg_.data();
    }

    constexpr String(std::string const& s) : String(s.data(), 0, s.size()) {}
    constexpr String(char const* src) : String(src, 0, detail::cstrlen(src)) {}

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
        detail::cmemcpy(chars, data(), size());
        detail::cmemcpy(chars + size(), rhs.data(), rhs.size());
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
    co_yield String(data, start, pos - start);
}

}  // namespace cxx

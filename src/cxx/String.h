#pragma once
#include <type_traits>
static_assert(__cplusplus >= 202300L, "cxx-libs requires C++23");
// (c) 2024 Steve O'Brien -- MIT License

#include "Generator.h"
#include "Util.h"
#include "detail/_string.h"

#include <cassert>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <string>
#include <utility>

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

    consteval void clearCEV() {
        if (isSmall()) {
            storage_.small_.clearCEV();
        } else {
            storage_.reg_.clearCEV();
        }
        size_ = 0;
    }

    constexpr void clear() {
        if consteval { return clearCEV(); }
        if (isSmall()) {
            storage_.small_.clear();
        } else {
            storage_.reg_.clear();
        }
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
    co_yield String(data, start, pos - start);
}

}  // namespace cxx

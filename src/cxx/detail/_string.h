#pragma once
static_assert(__cplusplus >= 202300L, "cxx-libs requires C++23");
// (c) 2024 Steve O'Brien -- MIT License

#include "../Util.h"

#include <atomic>
#include <cassert>
#include <cstddef>
#include <cstdlib>
#include <cstring>

namespace cxx::detail {

constexpr static char const* kEmpty = "";
constexpr static size_t kSmallMax = 15;

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
        auto bytes = sizeof(HeapString) + size;   // accomodates this struct + chars + NUL
        char* data = new char[bytes];             // home of our new HeapString
        auto* ret = (HeapString*) (data);         // ugly cast
        ret->rc_ = 0;                             // initial refcount is 1 (repr. as zero)
        ce_memcpy(&(ret->data_), src, size + 1);  // copy source string including its NUL
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

    consteval void clearCEV() { cev_memset(data_, char(0), sizeof(data_)); }
    constexpr void clear() { ce_memset(data_, char(0), sizeof(data_)); }
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

    consteval void clearCEV() { data_ = nullptr; }

    constexpr void clear() {
        data_ = nullptr;
        auto* hs = ptr_;
        ptr_ = nullptr;
        if (hs) { hs->release(); }
    }
};

}  // namespace cxx::detail

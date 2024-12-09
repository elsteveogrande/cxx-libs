#pragma once
static_assert(__cplusplus >= 202300L, "cxx-libs requires C++23");
// Copyright (c) 2024 Steve O'Brien
// MIT Licensed

#include "../Util.h"
#include "cxx/Ref.h"

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>

namespace cxx {

static constexpr int kDataSize = 16;

enum class Type : int { SMALL = 0, LITERAL = 1, SHARED = 2 };

struct [[gnu::packed]] Descriptor final {
    Type const type_ : 8;
    unsigned long const size_ : 56;

    constexpr static size_t ensureSize(size_t size) {
        assert(size < (1uz << 56));
        return size;
    }

    constexpr Descriptor() : type_(Type::SMALL), size_(0) {}

    constexpr Descriptor(Type type, size_t size)
            : type_(type), size_(ensureSize(size)) {}

    constexpr Descriptor(Descriptor const& rhs) = default;
};
static_assert(sizeof(Descriptor) == sizeof(size_t));

struct Small {
    static constexpr int kSmallLimit = kDataSize - sizeof(int64_t);
    static constexpr char const* kEmpty = "";

    char data_[kSmallLimit] {0};

    constexpr Small() = default;
    constexpr Small(char const* data, size_t size) {
        ce_memcpy(data_, data, size + 1);
    }

    constexpr Small(Small const& rhs) = default;
    constexpr Small& operator=(Small const& rhs) = default;

    char const* data() const { return data_; }
};
static_assert(sizeof(Small) <= kDataSize);

struct Literal {
    char const* data_;
    constexpr Literal(char const* data) : data_(data) {};
    char const* data() const { return data_; }
};
static_assert(sizeof(Literal) <= kDataSize);

struct SharedBuffer final : public RefCounted<SharedBuffer> {
    int64_t size_ {0};
    char data_;  // first char; this struct is over-allocated

    SharedBuffer(SharedBuffer const&) = delete;

    static Ref<SharedBuffer> create(size_t size, char const* data) {
        auto full = sizeof(SharedBuffer) + size;
        auto ret = Ref((SharedBuffer*) malloc(full));
        ret->size_ = size;
        ce_memcpy(&ret->data_, data, size);
        return ret;
    }

    char const* data() const { return &data_; }
};

struct Shared {
    Ref<SharedBuffer> contents;
};
static_assert(sizeof(Shared) <= kDataSize);

}  // namespace cxx

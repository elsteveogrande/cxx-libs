#pragma once
static_assert(__cplusplus >= 202300L, "cxx-libs requires C++23");
// Copyright (c) 2024 Steve O'Brien
// MIT Licensed

#include "../Util.h"
#include "_ref.h"

#include <array>
#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <utility>

namespace cxx::stringdetail {

enum class Type : int { TINY = 0, LITERAL = 1, SHARED = 2 };

struct SharedData final {
    int64_t size_ {0};
    detail::RefCount rc_;
    char cstr_;  // first char; this struct is over-allocated

    ~SharedData() = delete;
    SharedData(SharedData const&) = delete;

    static SharedData* create(size_t size, char const* src) {
        auto full = sizeof(SharedData) + size;
        auto* ret = (SharedData*) malloc(full);
        ret->size_ = size;
        // ret->rc_ = 1;  // already initially at 1
        ce_strncpy(&ret->cstr_, src, size);
        return ret;
    }

    void destroy() const {
        assert(!rc_);
        free((void*) this);
    }

    void take() const { rc_.inc(); }

    void drop() const {
        if (rc_.dec()) { destroy(); }
    }
};

static constexpr int kDataSize = 16;
static constexpr int kTinyLimit = kDataSize - sizeof(int64_t);

struct Tiny {
    std::array<char, kTinyLimit> chars;
};
struct Literal {
    char const* cstr;
};
struct Shared {
    SharedData* shared;
};

struct Data {
    int64_t size_;
    union {
        Tiny tiny;
        Literal lit;
        Shared sh;
    };

    constexpr Type type() const {
        if (size_ < 0) { return Type::SHARED; }
        if (size_ < 8) { return Type::TINY; }
        return Type::LITERAL;
    }

    constexpr size_t size() const { return abs(size_); }

    constexpr char const* cstr() const {
        switch (type()) {
        case Type::TINY:    return tiny.chars.data();
        case Type::LITERAL: return lit.cstr;
        case Type::SHARED:  return &sh.shared->cstr_;
        }
        std::unreachable();
    }

    constexpr static Data ofTiny(int64_t size, char const* cstr) {
        Data ret = {.size_ = size, .tiny = {}};
        ce_strncpy(ret.tiny.chars.data(), cstr, size + 1);
        return ret;
    }

    consteval static Data ofLiteral(int64_t size, char const* cstr) {
        return {.size_ = size, .lit = {.cstr = cstr}};
    }

    constexpr static Data ofShared(int64_t size, char const* cstr) {
        auto* shared = SharedData::create(size, cstr);
        return {.size_ = 0 - shared->size_, .sh = {shared}};
    }

    constexpr Data copy() const {
        if (type() == stringdetail::Type::SHARED) { sh.shared->take(); }
        return *this;
    }

    constexpr Data move() {
        Data ret = *this;
        memset(this, 0, sizeof(Data));
        return ret;
    }

    constexpr bool operator==(Data const& rhs) const {
        return (this == &rhs) ||
               (size() == rhs.size() && !memcmp(cstr(), rhs.cstr(), size()));
    }
};
static_assert(sizeof(Data) == kDataSize);

}  // namespace cxx::stringdetail

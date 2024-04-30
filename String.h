#pragma once
// Copyright (c) 2024 Steve O'Brien
// MIT Licensed

#include <atomic>
#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <cstring>

#include "Util.h"

namespace cxx {

namespace stringdetail {

enum class Type : int { TINY = 0, LITERAL = 1, SHARED = 2 };

struct SharedData final {
    int64_t size_ {0};
    [[gnu::aligned(8)]] uint64_t mutable uses_ {0};
    char cstr_;  // first char; this struct is over-allocated

    ~SharedData() = delete;
    SharedData(SharedData const&) = delete;

    static SharedData* create(size_t size, char const* src) {
        auto full = sizeof(SharedData) + size;
        SharedData* ret = (SharedData*) malloc(full);
        ret->size_ = size;
        ret->uses_ = 1;
        ce_strncpy(&ret->cstr_, src, size);
        return ret;
    }

    void destroy() const {
        assert(uses_ == 0);
        free((void*) this);
    }

    void take() const {
        auto& uses = *(std::atomic<uint64_t>*) (&uses_);
        ++uses;
    }

    void drop() const {
        auto& uses = *(std::atomic<uint64_t>*) (&uses_);
        if ((--uses) == 0) { destroy(); }
    }
};

static constexpr int kDataSize = 16;
static constexpr int kTinyLimit = kDataSize - sizeof(int64_t);

struct Tiny {
    char chars[kTinyLimit] {0};
};
struct Literal {
    char const* cstr;
};
struct Shared {
    SharedData* shared;
};

struct [[gnu::packed]] Data {
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
            case Type::TINY:
                return tiny.chars;
            case Type::LITERAL:
                return lit.cstr;
            case Type::SHARED:
                return &sh.shared->cstr_;
        }
        std::unreachable();
    }

    constexpr static Data ofTiny(int64_t size, char const* cstr) {
        Data ret = {.size_ = size, .tiny = {}};
        ce_strncpy(ret.tiny.chars, cstr, size + 1);
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
};
static_assert(sizeof(Data) == kDataSize);

}  // namespace stringdetail

struct String final {
    using Data = stringdetail::Data;

    stringdetail::Data data_;

    constexpr size_t size() const { return data_.size(); }
    constexpr char const* data() const { return data_.cstr(); }

    static constexpr Data fromCString(size_t size, char const* cstr) {
        if consteval {
            return (size < stringdetail::kTinyLimit)
                           ? Data::ofTiny(size, cstr)
                           : Data::ofLiteral(size, cstr);
        }
        return (size < stringdetail::kTinyLimit) ? Data::ofTiny(size, cstr)
                                                 : Data::ofShared(size, cstr);
    }

    constexpr ~String() {
        if consteval { return; }
        if (data_.type() == stringdetail::Type::SHARED) {
            data_.sh.shared->drop();
        }
    }

    constexpr String()
            : String(0, "") {}

    constexpr String(char const* cstr)
            : String(ce_strlen(cstr), cstr) {}

    constexpr String(size_t size, char const* cstr)
            : data_(fromCString(size, cstr)) {}

    constexpr String(String const& rhs)
            : data_(rhs.data_.copy()) {}

    String& operator=(String const& rhs) {
        *(const_cast<Data*>(&data_)) = rhs.data_.copy();
        return *this;
    }

    constexpr String(String&& rhs)
            : data_(rhs.data_.move()) {}

    String& operator=(String&& rhs) {
        *(const_cast<Data*>(&data_)) = rhs.data_.move();
        return *this;
    }

    constexpr String operator+(String const& rhs) const {
        auto total = size() + rhs.size();
        char tmp[total + 1];
        ce_strncpy(tmp, data(), size());
        ce_strncpy(tmp + size(), rhs.data(), rhs.size());
        tmp[total] = 0;
        return String(tmp);
    }

    constexpr char const& operator[](size_t i) const { return *(data() + i); }
};

}  // namespace cxx

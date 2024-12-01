#pragma once
static_assert(__cplusplus >= 202300L, "cxx-libs requires C++23");
// (c) 2024 Steve O'Brien -- MIT License

#include "Generator.h"
#include "Util.h"
#include "detail/_string.h"

#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <ostream>
#include <string>

namespace cxx {

struct String final {
    using Data = stringdetail::Data;

    stringdetail::Data data_;

    constexpr size_t size() const { return data_.size(); }
    constexpr char const* data() const { return data_.cstr(); }

    static constexpr char const* kEmptyCString = "";

    static consteval Data fromCStringCE(size_t size, char const* cstr) {
        return (size < stringdetail::kTinyLimit)
                       ? Data::ofTiny(size, cstr)
                       : Data::ofLiteral(size, cstr);
    }

    static constexpr Data fromCString(size_t size, char const* cstr) {
        if consteval { return fromCStringCE(size, cstr); }
        return (size < (stringdetail::kTinyLimit - 1))
                       ? Data::ofTiny(size, cstr)
                       : Data::ofShared(size, cstr);
    }

    constexpr ~String() {
        if consteval { return; }
        if (data_.type() == stringdetail::Type::SHARED) {
            data_.sh.shared->drop();
        }
    }

    constexpr String() : String(0, kEmptyCString) {}

    constexpr String(char const* cstr) : String(ce_strlen(cstr), cstr) {}

    constexpr String(size_t size, char const* cstr)
            : data_(fromCString(size, cstr)) {}

    constexpr String(String const& rhs) : data_(rhs.data_.copy()) {}

    constexpr String(std::string rhs) : String(rhs.data()) {}

    String& operator=(String const& rhs) {
        *(const_cast<Data*>(&data_)) = rhs.data_.copy();
        return *this;
    }

    constexpr String(String&& rhs) : data_(rhs.data_.move()) {}

    String& operator=(String&& rhs) {
        *(const_cast<Data*>(&data_)) = rhs.data_.move();
        return *this;
    }

    constexpr bool operator==(String const& rhs) const {
        return (this == &rhs) || (this->data_ == rhs.data_);
    }

    constexpr String operator+(String const& rhs) const {
        auto total = size() + rhs.size();
        char tmp[total + 1];
        ce_strncpy(tmp, data(), size());
        ce_strncpy(tmp + size(), rhs.data(), rhs.size());
        tmp[total] = 0;
        return {tmp};
    }

    constexpr operator std::string() const {
        std::string ret(data());
        return ret;
    }

    constexpr operator uint64_t() const {
        auto str = static_cast<std::string>(*this);
        return std::stoul(str);
    }

    constexpr char const& operator[](size_t i) const { return *(data() + i); }

    Generator<String> split(char sep) const {
        // TODO share a backing string and yield `SubString`s or something
        size_t const size = this->size();
        char const* data = this->data();
        size_t start = 0;  // current [initial] part starts here
        size_t pos = 0;    // current end position (exclusive)
        while (pos < size) {
            if (data[pos] == sep) {
                co_yield String(pos - start, data + start);
                start = pos + 1;  // skip past this char
            }
            ++pos;
        }
        co_yield String(pos - start, data + start);
    }
};
static_assert(sizeof(String) == stringdetail::kDataSize);

constexpr std::ostream& operator<<(std::ostream& os, cxx::String const& str) {
    return os << static_cast<std::string>(str);
}

}  // namespace cxx

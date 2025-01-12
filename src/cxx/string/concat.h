#pragma once
static_assert(__cplusplus >= 202300L, "cxx-libs requires C++23");
// (c) 2024 Steve O'Brien -- MIT License

#include "String.h"

#include <cstddef>

namespace cxx {

String String::operator+(String const& rhs) const {
    auto total = size() + rhs.size();
    char chars[total + 1];
    size_t i = 0;
    auto it = begin();
    auto end = this->end();
    while (it != end) { chars[i++] = *it++; }
    it = rhs.begin();
    end = rhs.end();
    while (it != end) { chars[i++] = *it++; }
    chars[total] = 0;
    return {chars, 0, total};
}

}  // namespace cxx

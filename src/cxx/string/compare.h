#pragma once
static_assert(__cplusplus >= 202300L, "cxx-libs requires C++23");
// (c) 2024 Steve O'Brien -- MIT License

#include "String.h"

namespace cxx {

constexpr bool String::operator<(char const* b) const {
    auto* a = data();
    for (; *a && *b; ++a, ++b) {
        if (*a < *b) { return true; }
    }
    return false;
}

constexpr bool String::operator<(String const& rhs) const { return *this < rhs.data(); }

constexpr bool String::operator==(char const* b) const {
    auto* a = data();
    for (; *a && *b; ++a, ++b) {
        if (*a != *b) { return false; }
    }
    return true;
}

constexpr bool String::operator==(String const& rhs) const { return *this == rhs.data(); }

}  // namespace cxx

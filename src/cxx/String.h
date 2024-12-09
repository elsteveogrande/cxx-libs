#pragma once
static_assert(__cplusplus >= 202300L, "cxx-libs requires C++23");
// (c) 2024 Steve O'Brien -- MIT License

#include "Generator.h"
#include "Util.h"

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <ostream>
#include <string>
#include <utility>

namespace cxx {

struct String final {
    enum class Type : int { SMALL = 0, LITERAL = 1, SHARED = 2 };
    Type type() const;
    size_t size() const;
    char const* data() const;
};

}  // namespace cxx

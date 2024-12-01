#pragma once
static_assert(__cplusplus >= 202300L, "cxx-libs requires C++23");
// (c) 2024 Steve O'Brien -- MIT License

#include "StackTrace.h"

#include <exception>

namespace cxx {

struct Exception : std::exception {
    cxx::StackTrace trace;
};

}  // namespace cxx

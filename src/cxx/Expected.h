#pragma once
static_assert(__cplusplus >= 202300L, "cxx-libs requires C++23");
// (c) 2024 Steve O'Brien -- MIT License

#include "Concepts.h"

namespace cxx {

// Declare types here so IDE considers this file (not a decl/ file) "authoritative"
template <typename T, class E>
    requires DifferentRCV<T, E> struct Expected;

}  // namespace cxx

#include "algo/Expected.h"

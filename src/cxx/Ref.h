#pragma once
static_assert(__cplusplus >= 202300L, "cxx-libs requires C++23");
// (c) 2024 Steve O'Brien -- MIT License

namespace cxx {

// Declare types here so IDE considers this file (not a decl/ file) "authoritative"
template <typename T>
struct Ref;

}  // namespace cxx

#include "ref/Ref.h"

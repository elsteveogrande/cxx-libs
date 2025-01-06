#pragma once
static_assert(__cplusplus >= 202300L, "cxx-libs requires C++23");
// (c) 2024 Steve O'Brien -- MIT License

namespace cxx {
template <class T>
struct Ref;
}

#include "decl/_Ref.h"

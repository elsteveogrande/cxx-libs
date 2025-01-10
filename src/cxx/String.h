#pragma once
static_assert(__cplusplus >= 202300L, "cxx-libs requires C++23");
// (c) 2024 Steve O'Brien -- MIT License

namespace cxx {
class String;
}

#include "decl/string/base.h"
#include "decl/string/compare.h"
#include "decl/string/concat.h"
#include "decl/string/ctor.h"
#include "decl/string/split.h"

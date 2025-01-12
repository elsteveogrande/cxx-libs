#pragma once
static_assert(__cplusplus >= 202300L, "cxx-libs requires C++23");
// (c) 2024 Steve O'Brien -- MIT License

namespace cxx {

// Declare types here so IDE considers this file (not a decl/ file) "authoritative"
class String;

}  // namespace cxx

#include "decl/string/base.h"
#include "decl/string/compare.h"
#include "decl/string/concat.h"
#include "decl/string/ctor.h"
#include "decl/string/split.h"

#include <cxx/Ref.h>

#pragma once
static_assert(__cplusplus >= 202300L, "cxx-libs requires C++23");
// (c) 2024 Steve O'Brien -- MIT License

namespace cxx {

// Declare types here so IDE considers this file (not a decl/ file) "authoritative"
class String;

}  // namespace cxx

#include "string/String.h"
#include "string/compare.h"
#include "string/concat.h"
#include "string/ctor.h"
#include "string/split.h"

#include <cxx/Ref.h>

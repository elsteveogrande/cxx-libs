#pragma once
static_assert(__cplusplus >= 202300L, "cxx-libs requires C++23");
// (c) 2024 Steve O'Brien -- MIT License

#include "_ObjectFile.h"
#include "_SourceLoc.h"

#include <cstdint>
#include <map>
#include <memory>

namespace cxx {

struct Section;
using SectionSP = std::shared_ptr<Section const>;

struct DWARF {
    static void
    evalLineProg(std::map<uintptr_t, SourceLoc>* out, SectionSP debugLine, SectionSP debugLineStr);
};

}  // namespace cxx

#pragma once
static_assert(__cplusplus >= 202300L, "cxx-libs requires C++23");
// (c) 2024 Steve O'Brien -- MIT License

#include "_SourceLoc.h"

#include <cerrno>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <cxxabi.h>
#include <dlfcn.h>
#include <execinfo.h>
#include <fcntl.h>
#include <map>
#include <memory>
#include <sys/fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

namespace cxx {

struct Section;
using SectionSP = std::shared_ptr<Section>;

struct DWARF {
    static void
    evalLineProg(std::map<uintptr_t, SourceLoc>* out, SectionSP debugLine, SectionSP debugLineStr);
};

}  // namespace cxx

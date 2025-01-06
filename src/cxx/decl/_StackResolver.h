#pragma once
static_assert(__cplusplus >= 202300L, "cxx-libs requires C++23");
// (c) 2024 Steve O'Brien -- MIT License

#include "_ObjectFile.h"
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
#include <string>
#include <sys/fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <vector>

namespace cxx {
struct StackResolver {
    void* dlHandle {nullptr};
    char const* dlError {nullptr};
    std::map<uintptr_t, SourceLoc> locs;

    // Note: quite a bit of state held here.  These are all managed by this Resolver
    // and are all deallocated when the resolver is destroyed.

    // Contains the program itself, extra DWARF debug files, shared libs, ...
    std::vector<BinarySP> binaries;

    ~StackResolver() {
        if (dlHandle) { dlclose(dlHandle); }
    }

    StackResolver() {
        dlHandle = dlopen(nullptr, RTLD_LAZY);
        dlError = dlerror();
        if (dlError) { return; }
    }

    void findBinaries(std::string const& thisProg);
    SourceLoc findLocation(uintptr_t addr, Binary& binary);
    SourceLoc findLocation(uintptr_t addr);
};

}  // namespace cxx

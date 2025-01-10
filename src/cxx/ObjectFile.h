#pragma once
#include <cstdint>
static_assert(__cplusplus >= 202300L, "cxx-libs requires C++23");
// (c) 2024 Steve O'Brien -- MIT License

#include "decl/_Bytes.h"
#include "decl/_File.h"
#include "decl/_MachO64.h"
#include "decl/_ObjectFile.h"
#include "decl/_SourceLoc.h"

#include <cstdlib>
#include <cxxabi.h>
#include <string>

namespace cxx {

BinarySP Binary::open(std::string const& path, uintptr_t vmaSlide) {
    auto file = File::open(path);
    if (!file) { return {}; }
    auto magic = file->cur().u32();
    switch (magic) {
    case 0xfeedfacf: return std::make_shared<MachOBinary64>(file, vmaSlide);
    default:         return {};
    }
}

// region helpers
std::string demangle(std::string const& sym) {
    std::string ret = sym;
    int st = 0;
    auto* demangled = abi::__cxa_demangle(sym.data(), nullptr, nullptr, &st);
    if (demangled && !st) {
        ret = demangled;
        free(demangled);
    }
    return ret;
}
// endregion

}  // namespace cxx

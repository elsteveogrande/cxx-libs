#pragma once
#include <cstdint>
static_assert(__cplusplus >= 202300L, "cxx-libs requires C++23");
// (c) 2024 Steve O'Brien -- MIT License

#include "decl/io/_Bytes.h"
#include "decl/io/_File.h"
#include "decl/prog/_MachO64.h"
#include "decl/prog/_ObjectFile.h"
#include "decl/prog/_SourceLoc.h"
#include "decl/ref/base.h"

#include <cstdlib>
#include <cxxabi.h>
#include <string>

namespace cxx {

Ref<Binary> Binary::open(std::string const& path, uintptr_t vmaSlide) {
    auto file = File::open(path);
    if (!file) { return {}; }
    auto magic = file->cur().u32();
    switch (magic) {
    case 0xfeedfacf: return Ref<MachOBinary64>::make(file, vmaSlide);
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

#include "Ref.h"

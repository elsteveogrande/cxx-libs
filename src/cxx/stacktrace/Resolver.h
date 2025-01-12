#pragma once
static_assert(__cplusplus >= 202300L, "cxx-libs requires C++23");
// (c) 2024 Steve O'Brien -- MIT License

#include "../prog/ObjectFile.h"
#include "../prog/SourceLoc.h"

#include <cstddef>
#include <cstdint>
#include <dlfcn.h>
#include <map>
#include <string>
#include <vector>

namespace cxx {

namespace detail {

/** Functions provided on OSX.  We'll look up these functions when this class is first used (failing
 * over to dummy funcs if not OSX).  This requires no headers or libraries. */
struct DYLD final {
    uint32_t imageCount() {
        static auto* func = (uint32_t (*)()) dlsym(RTLD_DEFAULT, "_dyld_image_count");
        return func ? func() : 0;
    }

    std::string getImageName(uint32_t index) {
        static auto* func = (char const* (*) (int) ) dlsym(RTLD_DEFAULT, "_dyld_get_image_name");
        return func ? func(index) : "";
    }

    uintptr_t getImageVMAddrSlide(uint32_t index) {
        static auto* func = (uintptr_t (*)(int)) dlsym(RTLD_DEFAULT, "_dyld_get_image_vmaddr_slide");
        return func ? func(index) : 0;
    }
};

}  // namespace detail

struct StackResolver final {
    std::map<uintptr_t, SourceLoc> locs;
    uintptr_t vmaSlide;  // for main program image only

    // Contains the program itself, extra DWARF debug files, shared libs, ...
    std::vector<Ref<Binary>> binaries;

    StackResolver() {
        detail::DYLD dyld;
        vmaSlide = dyld.getImageVMAddrSlide(0);
    }

    void findBinaries(std::string const& thisProg);
    SourceLoc findLocation(uintptr_t addr, Binary const& binary);
    SourceLoc findLocation(uintptr_t addr);
};

}  // namespace cxx

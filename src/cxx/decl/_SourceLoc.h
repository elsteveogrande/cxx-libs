#pragma once
static_assert(__cplusplus >= 202300L, "cxx-libs requires C++23");
// (c) 2024 Steve O'Brien -- MIT License

#include <cstdint>
#include <string>

namespace cxx {

struct Binary;

struct SourceLoc final {
    Binary const* binary;
    uintptr_t virtualAddr;  // for the main program, this is adjusted (see DYLD.getImageVMAddrSlide)
    std::string sourceFile {};
    unsigned line {0};
    unsigned col {0};
    operator bool() const { return binary && line; }
};

}  // namespace cxx

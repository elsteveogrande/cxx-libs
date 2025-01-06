#pragma once
static_assert(__cplusplus >= 202300L, "cxx-libs requires C++23");
// (c) 2024 Steve O'Brien -- MIT License

#include <cerrno>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <cxxabi.h>
#include <dlfcn.h>
#include <execinfo.h>
#include <fcntl.h>
#include <memory>
#include <string>
#include <sys/fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

namespace cxx {

struct Binary;
using BinarySP = std::shared_ptr<Binary>;

struct SourceLoc final {
    BinarySP binary;
    uintptr_t virtualAddr;
    Dl_info dlInfo {};
    std::string sourceFile {};
    unsigned line {0};
    unsigned col {0};
    operator bool() const { return binary && line; }
};

}  // namespace cxx

#pragma once
#include <cstdint>
#include <utility>
#include <vector>
static_assert(__cplusplus >= 202300L, "cxx-libs requires C++23");
// (c) 2024 Steve O'Brien -- MIT License

#include "_Bytes.h"
#include "_File.h"
#include "_SourceLoc.h"

#include <cerrno>
#include <cstddef>
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

struct Section;
using SectionSP = std::shared_ptr<Section>;
struct Binary;
using BinarySP = std::shared_ptr<Binary>;

/** A section has code / data / something else (like debug info, which is what we're after).
 * These are generally within segments (don't care about those). */
struct Section : Bytes {
    virtual ~Section() = default;
    virtual std::string name() = 0;
    virtual BinarySP binary() = 0;
};

/** A binary is a program, shared lib, or debug data.  It has segments (which have sections),
and is represented by a blob of bytes, which this (actually a subclass of this) will parse,
with the ultimate goal of providing a table of `SourceLoc`s. */
struct Binary : Bytes {
    FileSP file_;

    Binary(FileSP file) : file_(std::move(file)) {}
    virtual ~Binary() = default;
    virtual std::vector<SectionSP> sections() = 0;

    uint8_t const* data() const override { return file_->data(); }
    size_t size() const override { return file_->size(); }

    static BinarySP open(std::string const& path);
};

}  // namespace cxx

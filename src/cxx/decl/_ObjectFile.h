#pragma once
#include <cstdint>
static_assert(__cplusplus >= 202300L, "cxx-libs requires C++23");
// (c) 2024 Steve O'Brien -- MIT License

#include "_Bytes.h"
#include "_File.h"
#include "_SourceLoc.h"

#include <string>
#include <utility>
#include <vector>

namespace cxx {

struct Section;
using SectionSP = std::shared_ptr<Section const>;
struct Binary;
using BinarySP = std::shared_ptr<Binary const>;

/** A section has code / data / something else (like debug info, which is what we're after).
 * These are generally within segments (don't care about those). */
struct Section : Bytes {
    virtual ~Section() = default;
    virtual std::string name() const = 0;
    virtual Cursor contents() const = 0;
    virtual BinarySP binary() const = 0;
};

/** A binary is a program, shared lib, or debug data.  It has segments (which have sections),
and is represented by a blob of bytes, which this (actually a subclass of this) will parse,
with the ultimate goal of providing a table of `SourceLoc`s. */
struct Binary : Bytes {
    FileSP file_;
    uint64_t vmaSlide_;

    Binary(FileSP file, uintptr_t vmaSlide) : file_(std::move(file)), vmaSlide_(vmaSlide) {}
    virtual ~Binary() = default;
    virtual std::vector<SectionSP> sections() const = 0;

    static BinarySP open(std::string const& path, uintptr_t vmaSlide);
};

}  // namespace cxx

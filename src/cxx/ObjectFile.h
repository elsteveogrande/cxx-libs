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
#include <memory>
#include <string>

namespace cxx {

/*
    std::shared_ptr<Section> parseSection(Cursor cur, SegmentSP seg) {
        auto name = cur.fixedStr(16);     // @ 0
        auto segName = cur.fixedStr(16);  // @ 16 (ignored)
        auto vmaddr = cur.u64();          // @ 32
        auto vmsize = cur.u64();          // @ 40
        auto binaryOffset = cur.u32();    // @ 48
        cur += 28;                        // @ 52, skip the rest of this 80-byte record
        return std::make_shared<Section>(Section {.segment = seg,
                                                  .binaryOffset = binaryOffset,
                                                  .name = name,
                                                  .vmaddr = vmaddr,
                                                  .vmsize = vmsize});
    }

    void parseSections(this BinarySP self, Segment& segment, size_t nsects) {
        //
        https://github.com/aidansteele/osx-abi-macho-file-format-reference?tab=readme-ov-file#section_64
        // Start reading at the first section, just after the load-segment command.
        Cursor cur(self, segment.binaryOffset + 72);
        while (nsects--) {
            auto section = self->parseSection(cur, segment);
            segment.sections.push_back(section);
        }
    }
*/

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

#pragma once
#include <cxx/decl/_Bytes.h>
static_assert(__cplusplus >= 202300L, "cxx-libs requires C++23");
// (c) 2024 Steve O'Brien -- MIT License

#include "decl/_File.h"
#include "decl/_MachO64.h"
#include "decl/_ObjectFile.h"
#include "decl/_SourceLoc.h"

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

    std::shared_ptr<Segment> parseSegmentFields(Cursor cur) {
        // Starting at offset 8 (we consumed cmd, cmdSize), read up to 56 (filesize)
        auto binaryOffset = cur.offset_ - 8;
        auto name = cur.fixedStr(16);  // @ 8
        auto vmaddr = cur.u64();       // @ 24
        auto vmsize = cur.u64();       // @ 32
        auto fileoff = cur.u64();      // @ 40
        auto filesize = cur.u64();     // @ 48
        printf("Seg %zx %s addr=%llx size=%llx off=%llx fsz=%llx\n",
               binaryOffset,
               name.data(),
               vmaddr,
               vmsize,
               fileoff,
               filesize);
        return std::make_shared<Segment>(Segment {.binary = *this,
                                                  .binaryOffset = binaryOffset,
                                                  .name = name,
                                                  .vmaddr = vmaddr,
                                                  .vmsize = vmsize,
                                                  .fileoff = fileoff,
                                                  .filesize = filesize,
                                                  .sections = {}});
    }

    void parseSegment64(Cursor cur) {
        //
        https://github.com/aidansteele/osx-abi-macho-file-format-reference?tab=readme-ov-file#segment_command_64
        auto segment = parseSegmentFields(cur);
        segments.push_back(segment);
        cur += 8;                         // @ 56: skip
        auto nsects = cur.u32();          // @ 64
        cur += 4;                         // @ 68
        parseSections(*segment, nsects);  // first section @ 72
    }

    void parseLoadCommands(Cursor cur, uint32_t count) {
        while (count--) {
            auto cmd = cur.u32();
            printf("command @ %zx : %02x\n", cur.offset_ - 4, cmd);
            auto size = cur.u32();
            if (!size) {
                std::cerr << "[StackTrace] Command size apparently 0, Mach-O parser probably "
                             "broken"
                          << std::endl;
                return;
            }
            // TODO: LC_SEGMENT -> parseSegment32
            // TODO: LC_UUID (to match against the dSYM file)
            if (cmd == 0x19) { parseSegment64(cur); }
            cur += size - 8;  // reposition cursor at next segment regardless
        }
    }

    void parseMachO() {
        //
        https://github.com/aidansteele/osx-abi-macho-file-format-reference?tab=readme-ov-file#header-data-structure
        Cursor cur(*this, 0x10);  // start @ 0x10
        auto ncmds = cur.u32();   // @ 0x10: number of load commands
        cur = 0x68;               // @ 0x68: load commands start
        parseLoadCommands(cur, ncmds);
    }
*/

BinarySP Binary::open(std::string const& path) {
    auto file = File::open(path);
    if (!file) { return {}; }

    auto magic = Cursor(*file, 0).u32();
    switch (magic) {
    case 0xfeedfacf: return std::make_shared<MachOBinary64>(file);
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

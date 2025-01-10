#pragma once
static_assert(__cplusplus >= 202300L, "cxx-libs requires C++23");
// (c) 2024 Steve O'Brien -- MIT License

#include "_Bytes.h"
#include "_Cursor.h"
#include "_File.h"
#include "_ObjectFile.h"
#include "_SourceLoc.h"

#include <cstddef>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

namespace cxx {

struct MachOSection64;
using MachOSection64SP = std::shared_ptr<MachOSection64 const>;
struct MachOBinary64;
using MachOBinary64SP = std::shared_ptr<MachOBinary64 const>;

struct MachOSection64 final : Section {
    constexpr static size_t kSectionHeaderSize = 80;
    MachOBinary64SP binary_;
    Cursor const base_;

    MachOSection64(MachOBinary64SP binary, Cursor base)
            : binary_(std::move(binary))
            , base_(std::move(base)) {}

    Cursor cur() const override { return base_; }
    std::string name() const override;
    BinarySP binary() const override;
    Cursor contents() const override;
};

struct MachOBinary64 final : Binary, std::enable_shared_from_this<MachOBinary64> {
    size_t loadCommands_;

    MachOBinary64(FileSP file, uintptr_t vmaSlide);
    Cursor cur() const override;

    std::vector<SectionSP> sections() const override;
    static MachOBinary64 open(std::string path);
};

/*
    mach_header_64: (32 bytes)
    https://github.com/aidansteele/osx-abi-macho-file-format-reference?tab=readme-ov-file#mach_header_64
    +0                   +4                   +8                   +12
    | 0xfeedfacf (magic) | (@ 4) cputype      | (@ 8) cpusubtype   | (@ 12) filetype    |
    | (@ 16) ncmds       | (@ 20) sizeofcmds  | (@ 24) flags       | (@ 28) reserved    |
*/
MachOBinary64::MachOBinary64(FileSP file, uintptr_t vmaSlide)
        : Binary(file, vmaSlide)
        , loadCommands_((cur() + 16).u32()) {}

Cursor MachOBinary64::cur() const { return this->file_->cur(); }

/*
    Segment layout, 72 (0x48) byte header, followed by its sections:
    https://github.com/aidansteele/osx-abi-macho-file-format-reference?tab=readme-ov-file#segment_command_64
    +0                   +4                   +8                   +12
    | LC_SEGMENT_64=0x19 | (@ 4) cmdsize      | (@ 8) segment name string (16 chars...) :
    : (... name continued)                    | (@ 24) vmaddr                           |
    | (@ 32) vmsize                           | (@ 40) fileoff                          |
    | (@ 48) filesize                         | (@ 56) maxprot     | (@ 60) initprot    |
    | (@ 64) nsects      | (@ 68) flags       |   { ... followed by its sections ... }
*/

std::vector<SectionSP> MachOBinary64::sections() const {
    constexpr static uint32_t kLoadSegment64 = 0x19;

    std::vector<SectionSP> ret;

    // Iterate through load commands to find segments and their contained sections.
    // Load commands are immediately after the header
    auto cur = this->cur() + 0x20;

    auto cmds = loadCommands_;
    while (cmds--) {
        auto cmd = cur.u32();
        auto cmdSize = cur.u32();
        if (cmd == kLoadSegment64) {
            cur += 56;
            auto nsects = cur.u32();
            cur += 4;
            while (nsects--) {
                ret.push_back(std::make_shared<MachOSection64>(shared_from_this(), cur));
                cur += MachOSection64::kSectionHeaderSize;
            }
        } else {
            cur += (cmdSize - 8);
        }
    }

    return ret;
}

/*
    Section layout, 80 (0x50) bytes:
    https://github.com/aidansteele/osx-abi-macho-file-format-reference?tab=readme-ov-file#section_64
    +0                   +4                   +8                   +12
    | (@ 0) section name (16 bytes)                                                     |
    | (@ 16) segment name (16 bytes)                                                    |
    | (@ 32) addr                             | (@ 40) size                             |
    | (@ 48) offset      | (@ 52) align       | (@ 56) reloff      | (@ 60) nreloc      |
    | (@ 64) flags       | (@ 68) reserved  . . .                                       |
*/

std::string MachOSection64::name() const { return cur().fixedStr(16); }

BinarySP MachOSection64::binary() const { return binary_; }

Cursor MachOSection64::contents() const {
    size_t offset = (cur() + 48).u32();
    return (binary()->cur() + offset);
}

}  // namespace cxx

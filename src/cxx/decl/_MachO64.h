#pragma once
#include <cxx/decl/_Bytes.h>
#include <cxx/decl/_File.h>
#include <vector>
static_assert(__cplusplus >= 202300L, "cxx-libs requires C++23");
// (c) 2024 Steve O'Brien -- MIT License

#include "_ObjectFile.h"
#include "_SourceLoc.h"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>

namespace cxx {

// https://github.com/aidansteele/osx-abi-macho-file-format-reference?tab=readme-ov-file#segment_command_64

struct MachOSection64;
using MachOSection64SP = std::shared_ptr<MachOSection64>;
struct MachOBinary64;
using MachOBinary64SP = std::shared_ptr<MachOBinary64>;

struct MachOSection64 final : Section {
    constexpr static size_t kSectionHeaderSize = 80;
    MachOBinary64SP binary_ {};
    size_t const offset_ {};

    size_t size() const override { return kSectionHeaderSize; }
    BinarySP binary() override;
    uint8_t const* data() const override;
    std::string name() override;
};

struct MachOBinary64 final : Binary {
    size_t loadCommands_;
    MachOBinary64(FileSP file) : Binary(file), loadCommands_(Cursor(*this, 16).u32()) {}
    std::vector<SectionSP> sections() override;
    static MachOBinary64 open(std::string path);
};

BinarySP MachOSection64::binary() { return binary_; }
uint8_t const* MachOSection64::data() const { return binary_->data() + offset_; }

std::string MachOSection64::name() { return Cursor {*this, 0}.fixedStr(16); }

std::vector<SectionSP> MachOBinary64::sections() {
    // Segments:
    // https://github.com/aidansteele/osx-abi-macho-file-format-reference?tab=readme-ov-file#segment_command_64
    // Sections:
    // https://github.com/aidansteele/osx-abi-macho-file-format-reference?tab=readme-ov-file#section
    std::vector<SectionSP> ret;
    Cursor cur {*this, 0x68};  // past header; is location of first segment
    return ret;
}

}  // namespace cxx

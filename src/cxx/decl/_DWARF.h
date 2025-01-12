#pragma once
static_assert(__cplusplus >= 202300L, "cxx-libs requires C++23");
// (c) 2024 Steve O'Brien -- MIT License

#include "_Cursor.h"
#include "_ObjectFile.h"
#include "_SourceLoc.h"
#include "ref/base.h"

#include <cstddef>
#include <cstdint>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace cxx {

struct Section;

struct DWARF {
    static void
    evalLineProg(std::map<uintptr_t, SourceLoc>* out, Ref<Section> debugLine, Ref<Section> debugLineStr);
};

}  // namespace cxx

namespace cxx {

namespace detail {

void getFilenames(std::vector<std::string>* files, Cursor& data, Cursor const& stringData) {
    auto str = [&stringData](size_t offset) { return (stringData + offset).str(); };

    data.u8(0x01, true, "dirFormatCount");
    data.u8(0x01, true, "dirFormat");
    data.u8(0x1f, true, "dirFormatForm");
    uint64_t dirCount = data.uleb();
    std::vector<std::string> dirs;
    while (dirCount--) { dirs.push_back(str(data.u32())); }

    data.u8(0x02, true, "fileFormatCount");
    data.u8(0x01, true, "fileFormat1");
    data.u8(0x1f, true, "fileFormat1Form");
    data.u8(0x02, true, "fileFormat2");
    data.u8(0x0b, true, "fileFormat2Form");

    uint64_t fileCount = data.uleb();
    while (fileCount--) {
        auto strID = data.u32();
        auto dirID = data.uleb();
        auto file = str(strID);
        if (file != "<stdin>") { file = dirs[dirID] + "/" + file; }
        files->push_back(file);
    }
}

}  // namespace detail

void DWARF::evalLineProg(
        std::map<uintptr_t, SourceLoc>* out, Ref<Section> debugLine, Ref<Section> debugLineStr) {
    auto data = debugLine->contents();
    auto const stringData = debugLineStr->contents();

    auto binary = debugLine->binary();

    auto length = data.u32(0xffffffff, false, "DWARF64 not supported");
    auto const dataLimit = data + length;
    data.u16(0x0005, true, "version");
    data.u8(0x08, true, "address_size");
    data.u8(0x00, true, "seg_select_size");
    auto prologueSize = data.u32();
    data.u8(0x01, true, "min_inst_length");
    data.u8(0x01, true, "max_ops_per_inst");
    data += 1;

    auto u8 = [&data]() { return data.u8(); };
    auto u16 = [&data]() { return data.u16(); };
    auto u32 = [&data]() { return data.u32(); };
    int lineBase = data.i8();
    unsigned lineRange = data.u8();
    unsigned opcodeBase = data.u8();
    std::vector<size_t> opSizes {0};  // [0] is 0
    for (size_t i = 1; i < opcodeBase; i++) { opSizes.push_back(u8()); /* init 1 through 12 */ }

    std::vector<std::string> files;
    detail::getFilenames(&files, data, stringData);

    // Subset of "VM registers"
    uint64_t addr;
    int32_t fileIndex;
    uint32_t line;
    uint32_t col;

    // Initial state
    auto reset = [&]() {
        addr = 0;
        fileIndex = 1;
        line = 1;
        col = 0;
    };
    reset();

    // Start interpreting commands

    std::stringstream err;

    auto append = [&]() {
        auto sourceFile = (fileIndex >= 0) ? files[fileIndex] : "";
        // std::printf("addr:%016llx file:%s line:%d col:%d\n", addr, sourceFile.data(), line, col);
        (*out)[addr] =
                {.binary = binary, .virtualAddr = addr, .sourceFile = sourceFile, .line = line, .col = col};
    };

    auto doStandard = [&](uint8_t op) {
        // std::printf("op %02x standard\n", op);
        auto opSize = opSizes[op];
        uint8_t adjOp, opAdv;
        switch (op) {
        case 0x01:                      // DW_LNS_copy
            append();                   // emit
            break;                      //
        case 0x02:                      // DW_LNS_advance_pc
            addr += data.uleb();        // advance
            break;                      //
        case 0x03:                      // DW_LNS_advance_line
            line += data.sleb();        // advance (or go back)
            break;                      //
        case 0x04:                      // DW_LNS_set_file
            fileIndex = data.uleb();    // set to uleb file index
            break;                      //
        case 0x05:                      // DW_LNS_set_column
            col = data.uleb();          // set new val
            break;                      //
        case 0x08:                      // DW_LNS_const_add_pc
            adjOp = 255 - opcodeBase;   // incr addr (but not line), as
            opAdv = adjOp / lineRange;  // if special opcode 255 (p163),
            addr += opAdv;              // assuming maxInsnOps == 1 (asserted above)
            break;                      //
        case 0x09:                      // DW_LNS_fixed_advance_pc
            addr += data.u16();         // unencoded "uhalf"
            break;                      //
        case 0x06:                      // DW_LNS_negate_stmt
        case 0x07:                      // DW_LNS_set_basic_block
        case 0x0a:                      // DW_LNS_set_prologue_end
        case 0x0b:                      // DW_LNS_set_epilogue_begin
        case 0x0c:                      // DW_LNS_set_isa
            break;                      //
        default: err << "doStandard op=" << unsigned(op); throw std::runtime_error(err.str());
        }
    };

    auto doExtended = [&]() {
        auto opSize = u8();
        auto eop = u8();
        // std::printf("op %02x extended\n", eop);
        switch (eop) {
        case 0x01:              // DW_LNE_end_sequence
            line = 0;           // sentinel marker
            col = 0;            //
            fileIndex = -1;     //
            append();           // send out last record
            reset();            // back to the initial state
            break;              //
        case 0x02:              // DW_LNE_set_address
            addr = data.u64();  // read 64bit addr
            break;              //
        case 0x03:              // DW_LNE_set_discriminator
            data.uleb();        // ignore
            break;              //
        default: {
            err << "doExtended op=" << unsigned(eop);
            throw std::runtime_error(err.str());
        }
        }
    };

    auto doSpecial = [&](uint8_t op) {
        // std::printf("op %02x special\n", op);
        // https://dwarfstd.org/doc/DWARF5.pdf (p161)
        auto adjOp = op - opcodeBase;
        auto opAdv = adjOp / lineRange;
        auto lineAdv = lineBase + (adjOp % lineRange);
        addr += opAdv;  // assuming maxInsnOps == 1 (asserted above)
        line += lineAdv;
        append();
    };

    auto doNextOp = [&]() {
        err.clear();
        auto op = u8();
        if (op == 0) { return doExtended(); }
        if (op < opcodeBase) { return doStandard(op); }
        return doSpecial(op);
    };

    while (data < dataLimit) { doNextOp(); }

    fileIndex = -1;
    line = 0;
    col = 0;
    append();
}

}  // namespace cxx

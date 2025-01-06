#pragma once
static_assert(__cplusplus >= 202300L, "cxx-libs requires C++23");
// (c) 2024 Steve O'Brien -- MIT License

#include "decl/_Cursor.h"
#include "decl/_DWARF.h"
#include "decl/_Exception.h"
#include "decl/_ObjectFile.h"
#include "decl/_SourceLoc.h"
#include "decl/_StackFrame.h"
#include "decl/_StackResolver.h"
#include "decl/_StackTrace.h"

#include <cstddef>
#include <cstdint>
#include <dlfcn.h>
#include <exception>
#include <iostream>
#include <map>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace cxx {

struct StackTrace;
struct StackFrame;
struct StackResolver;

void StackResolver::findBinaries(std::string const& thisProg) {
    // minor TODO: looks like `binaries` gets some dupes.

    BinarySP bin;

    // If the program itself can be opened, add that
    bin = Binary::open(thisProg, vmaSlide);
    if (bin) { binaries.push_back(bin); }

    // Look for the debugging data under `dSYM` (OSX).
    // E.g.: for the program:
    //   [...]/build/StackTraceTests.asan
    // we look for the file:
    //   [...]/build/StackTraceTests.asan.dSYM/Contents/Resources/DWARF/StackTraceTests.asan
    size_t slash = 0;
    while (true) {
        auto nextSlash = thisProg.find('/', slash + 1);
        if (nextSlash == std::string::npos) { break; }
        slash = nextSlash;
    }
    auto progName = thisProg.substr(slash + 1);
    std::stringstream dwarfName;
    dwarfName << thisProg << ".dSYM/Contents/Resources/DWARF/" << progName;
    bin = Binary::open(dwarfName.str(), vmaSlide);
    if (bin) { binaries.push_back(bin); }
}

void getLocations(std::map<uintptr_t, SourceLoc>* out, Binary const& binary) {
    SectionSP debugLine;
    SectionSP debugLineStr;
    for (auto section : binary.sections()) {
        if (section->name() == ".debug_line") { debugLine = section; }
        if (section->name() == "__debug_line") { debugLine = section; }
        if (section->name() == ".debug_line_str") { debugLineStr = section; }
        if (section->name() == "__debug_line_str") { debugLineStr = section; }
    }
    if (debugLine && debugLineStr) {
        try {
            DWARF::evalLineProg(out, debugLine, debugLineStr);
        } catch (std::exception const& e) {
            std::cerr << "cannot decode DWARF: " << e.what() << std::endl;
        }
    }
}

SourceLoc StackResolver::findLocation(uintptr_t addr, Binary const& binary) {
    getLocations(&locs, binary);
    if (locs.size()) {
        addr -= binary.vmaSlide_;
        auto it = locs.upper_bound(addr);
        --it;
        if (addr >= it->second.virtualAddr) { return it->second; }
    }
    return {};
}

SourceLoc StackResolver::findLocation(uintptr_t addr) {
    for (auto& bin : binaries) {
        auto ret = findLocation(addr, *bin);
        if (ret) { return ret; }
    }
    return {};
}

void StackFrame::resolve(StackResolver& sr) const {
    // Use basic DL calls available in [g]libc.
    // This gets us the symbol and binary file (this program or shared lib).
    if (filename.empty() || symbol.empty()) {
        Dl_info info;
        dladdr(address, &info);
        if (dlerror()) { return; }
        if (info.dli_fname) { filename = info.dli_fname; }
        if (info.dli_sname) {
            symbol = info.dli_sname;
            demangled = demangle(symbol.data());
        }
    }

    // Try to locate this binary and/or companion DWARF files
    // (.dwo files for separated debug info, or `.dSYM/` dirs on OSX).
    sr.findBinaries(filename);

    // Search `binaries` for DWARF entries indication source locations
    this->loc = sr.findLocation((uintptr_t) address);
}

StackTrace::StackTrace() {
    void* frameAddr = __builtin_frame_address(0);     // Starting at current frame,
    auto* nextFramePtr = &this->frame;                // build linked list of frames.
    while (frameAddr) {                               // Until we hit the end...:
        void** ptr = (void**) frameAddr;              // start poking around in the stack
        auto ipNext = ((uintptr_t) (ptr[1]));         // find IP of instruction after call
        if (!ipNext) { break; }                       // (if no return address, we're at end).
        void* ip = (void*) (ipNext - 1);              // Back up to last byte of prev instruction
        auto frame = std::make_shared<StackFrame>();  // create a new frame
        frame->address = ip;                          // set address to (near) the calling insn
        *nextFramePtr = frame;                        // append frame to the list
        nextFramePtr = &frame->next;                  // ready to append next frame (if any)
        frameAddr = ptr[0];                           // continue walking the stack
    }
}

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

void DWARF::evalLineProg(std::map<uintptr_t, SourceLoc>* out, SectionSP debugLine, SectionSP debugLineStr) {
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
    getFilenames(&files, data, stringData);

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

#include <cxx/Exception.h>
#include <cxx/ObjectFile.h>

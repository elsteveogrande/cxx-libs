#pragma once
static_assert(__cplusplus >= 202300L, "cxx-libs requires C++23");
// (c) 2024 Steve O'Brien -- MIT License

#include "exc/Exception.h"
#include "prog/DWARF.h"
#include "prog/ObjectFile.h"
#include "prog/SourceLoc.h"
#include "ref/Ref.h"
#include "stacktrace/Frame.h"
#include "stacktrace/Resolver.h"
#include "stacktrace/Trace.h"

#include <cstddef>
#include <cstdint>
#include <dlfcn.h>
#include <exception>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

namespace cxx {

// Declare types here so IDE considers this file (not a decl/ file) "authoritative"

struct StackTrace;
struct StackFrame;
struct StackResolver;

void StackResolver::findBinaries(std::string const& thisProg) {
    // minor TODO: looks like `binaries` gets some dupes.

    Ref<Binary> bin;

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
    Ref<Section> debugLine;
    Ref<Section> debugLineStr;
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
    void* frameAddr = __builtin_frame_address(0);  // Starting at current frame,
    auto* nextFramePtr = &this->frame;             // build linked list of frames.
    while (frameAddr) {                            // Until we hit the end...:
        void** ptr = (void**) frameAddr;           // start poking around in the stack
        auto ipNext = ((uintptr_t) (ptr[1]));      // find IP of instruction after call
        if (!ipNext) { break; }                    // (if no return address, we're at end).
        void* ip = (void*) (ipNext - 1);           // Back up to last byte of prev instruction
        auto frame = Ref<StackFrame>::make();      // create a new frame
        frame->address = ip;                       // set address to (near) the calling insn
        *nextFramePtr = frame;                     // append frame to the list
        nextFramePtr = &frame->next;               // ready to append next frame (if any)
        frameAddr = ptr[0];                        // continue walking the stack
    }
}

}  // namespace cxx

#include <cxx/Exception.h>
#include <cxx/ObjectFile.h>

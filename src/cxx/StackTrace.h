#pragma once
static_assert(__cplusplus >= 202300L, "cxx-libs requires C++23");
// (c) 2024 Steve O'Brien -- MIT License

#include <cstdint>
#include <execinfo.h>
#include <iomanip>
#include <iostream>
#include <memory>
#include <sstream>

namespace cxx {

struct StackFrame final {
    void const* address;
    char symbol[256];
    char filename[128];
    int32_t line;
    std::shared_ptr<StackFrame> next {};

    void dump() const noexcept;
};

struct StackTrace final {
    static unsigned const CHOP = 2;
    std::shared_ptr<StackFrame> frame {};
    [[gnu::noinline]] StackTrace();
    void dump() const noexcept;
};

void StackFrame::dump() const noexcept {
    std::stringstream loc;
    loc << filename << ':' << line;
    std::cerr << "... 0x" << std::setw(16) << std::setfill('0') << std::hex << uint64_t(address)
              << ' ' << std::setw(32) << std::setfill(' ') << loc.str() << ' ' << std::setw(0)
              << std::setw(0) << symbol << std::endl;
}

[[gnu::noinline]] StackTrace::StackTrace() {
    void* frameAddr = __builtin_frame_address(0);
    auto chop = CHOP;
    std::shared_ptr<StackFrame> prev {nullptr};
    while (frameAddr) {
        if (chop--) { continue; }
        void** ptr = (void**) frameAddr;
        void* ip = ptr[1];
        auto frame = std::make_shared<StackFrame>();
        frame->address = ip;
        frame->symbol[0] = 0;
        frame->filename[0] = 0;
        frame->line = 0;
        if (prev) {
            prev->next = frame;
        } else {
            this->frame = frame;
        }
        prev = frame;

        frameAddr = ptr[0];
    }
}

void StackTrace::dump() const noexcept {
    auto f = this->frame;
    while (f) {
        f->dump();
        f = f->next;
    }
    std::cerr.flush();
}

}  // namespace cxx

#pragma once
static_assert(__cplusplus >= 202300L, "cxx-libs requires C++23");
// (c) 2024 Steve O'Brien -- MIT License

#include "_StackFrame.h"
#include "_StackResolver.h"
#include "ref/base.h"

#include <cstdint>
#include <cstdlib>
#include <cxxabi.h>
#include <iomanip>
#include <iostream>
#include <string>

namespace cxx {

std::string demangle(char const* name) {
    int st = 0;
    auto* demangled = abi::__cxa_demangle(name, nullptr, nullptr, &st);
    if (!demangled) { return {name}; }
    std::string ret = {demangled};
    free(demangled);
    return ret;
}

struct StackTrace final {
    // clang-format off
    Ref<StackFrame> frame {};
    StackTrace();

    using value_type = StackFrame;
    using iterator = StackFrameIterator;
    iterator begin() const { return {frame.get()}; }
    iterator end() const { return {nullptr}; }

    void resolve(StackResolver& sr) const { for (auto* sf : *this) { sf->resolve(sr); } }
 
    void dump(std::ostream& os) const {
        // These lines can unfortunately get rather long, so we take care to
        // trim some crap: shorten the filenames, but add only enough padding to make
        // things line up nicely.  See also the `loc()` method
        auto end = this->end();
        auto it = this->begin();
        // If first entry is for `cxx::StackTrace::StackTrace()`, discard
        if (it != end && (*it)->symbol == "_ZN3cxx10StackTraceC1Ev") { ++it; }
        auto begin = it;
        int width = 15;  // minimum width; arbitrary
        auto adjust = [&width] (int w) { width = (w > width) ? w : width; };
        while (it != end) { adjust((*it)->locStr().size()); ++it; }  // find max width for filename
        if (width > 40) { width = 40; }
        it = begin;
        while (it != end) {
            auto& sf = **it;
            os << "... 0x" << std::setw(12) << std::setfill('0') << std::hex << uint64_t(sf.address)
            << ' ' << std::setw(width) << std::setfill(' ') << sf.locStr()
            << ' ' << std::setw(0) << sf.sym() << std::endl;
            ++it;
        }
    }
    // clang-format on
};

}  // namespace cxx

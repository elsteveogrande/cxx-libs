#pragma once
static_assert(__cplusplus >= 202300L, "cxx-libs requires C++23");
// (c) 2024 Steve O'Brien -- MIT License

#include "_SourceLoc.h"
#include "_StackResolver.h"

#include <cerrno>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <cxxabi.h>
#include <dlfcn.h>
#include <execinfo.h>
#include <fcntl.h>
#include <filesystem>
#include <sstream>
#include <string>
#include <sys/fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

namespace cxx {

struct StackFrame final {
    void const* address {nullptr};
    std::shared_ptr<StackFrame> next {};
    // All these are mutable and only filled in when `resolve` is called,
    // so `resolve` can be used within a `catch (cxx::Exception const& ...)`
    std::string mutable symbol {};
    std::string mutable demangled {};
    std::string mutable filename {};
    SourceLoc mutable loc {};

    std::string sym() const { return (!demangled.empty() ? demangled : symbol); }

    std::string locStr() const {
        std::stringstream ret;
        std::string file = (loc.sourceFile.empty() ? filename : loc.sourceFile);
        std::string rel = std::filesystem::relative(file);
        if (rel[0] != '/' && rel[0] != '.') { rel = std::string("./") + rel; }
        ret << (rel.size() < file.size() ? rel : file);
        if (loc.line) { ret << ':' << loc.line; }
        if (loc.col) { ret << ':' << loc.col; }
        return ret.str();
    }

    void resolve(StackResolver& resolver) const;
};

struct StackFrameIterator {
    // clang-format off
    StackFrame* ptr;
    StackFrame* operator*() { return ptr; }
    StackFrameIterator& operator++() { ptr = ptr->next.get(); return *this; }
    StackFrameIterator& operator++(int) { ptr = ptr->next.get(); return *this; }
    bool operator==(StackFrameIterator const& rhs) const { return (!ptr && !rhs.ptr); }
    // clang-format on
};

}  // namespace cxx

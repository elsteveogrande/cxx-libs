#pragma once
#include <memory>
static_assert(__cplusplus >= 202300L, "cxx-libs requires C++23");
// (c) 2024 Steve O'Brien -- MIT License

#include "_Bytes.h"

#include <cerrno>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cxxabi.h>
#include <dlfcn.h>
#include <execinfo.h>
#include <fcntl.h>
#include <string>
#include <sys/fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

namespace cxx {

struct File;
using FileSP = std::shared_ptr<File>;

struct File : Bytes {
    std::string path_;
    int const fd_;
    size_t const size_;
    void const* const mmap_;

    virtual ~File() {
        ::close(fd_);
        ::munmap((void*) mmap_, size_);
    }

    uint8_t const* data() const override { return (uint8_t const*) mmap_; }
    size_t size() const override { return size_; }

    File(std::string path)
            : path_(path)
            , fd_(::open(path.data(), O_RDONLY))
            , size_(fd_ == -1 ? 0 : ::lseek(fd_, 0, SEEK_END))
            , mmap_(fd_ == -1 ? nullptr : ::mmap(nullptr, size_, PROT_READ, MAP_SHARED, fd_, 0)) {}

    static FileSP open(std::string path) {
        auto ret = std::make_shared<File>(path);
        if (ret->size()) { return ret; }
        return {};
    }
};

}  // namespace cxx

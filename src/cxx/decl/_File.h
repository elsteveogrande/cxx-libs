#pragma once
static_assert(__cplusplus >= 202300L, "cxx-libs requires C++23");
// (c) 2024 Steve O'Brien -- MIT License

#include "_Bytes.h"
#include "_Cursor.h"

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <fcntl.h>
#include <memory>
#include <string>
#include <sys/fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

namespace cxx {

struct File;
using FileSP = std::shared_ptr<File const>;

struct File : Bytes, std::enable_shared_from_this<File> {
    std::string path_;
    int const fd_;
    size_t const size_;
    void const* const mmap_;

    virtual ~File() {
        ::close(fd_);
        ::munmap((void*) mmap_, size_);
    }

    bool valid() const { return fd_ != -1; }

    Cursor cur() const override;

    File(std::string path)
            : path_(path)
            , fd_(::open(path.data(), O_RDONLY))
            , size_(fd_ == -1 ? 0 : ::lseek(fd_, 0, SEEK_END))
            , mmap_(fd_ == -1 ? nullptr : ::mmap(nullptr, size_, PROT_READ, MAP_SHARED, fd_, 0)) {}

    static FileSP open(std::string path) {
        auto ret = std::make_shared<File>(path);
        if (ret->valid()) { return ret; }
        return {};
    }
};

Cursor File::cur() const { return {shared_from_this(), (uint8_t const*) mmap_, size_}; }

}  // namespace cxx

#pragma once
static_assert(__cplusplus >= 202300L, "cxx-libs requires C++23");
// (c) 2024 Steve O'Brien -- MIT License

#include "../../Generator.h"
#include "base.h"

namespace cxx {

Generator<String> String::split(char sep) const {
    size_t const size = this->size();
    char const* data = this->data();
    size_t start = 0;  // current [initial] part starts here
    size_t pos = 0;    // current end position (exclusive)
    for (; pos < size; ++pos) {
        if (data[pos] == sep) {
            co_yield String(data, start, pos - start);
            start = pos + 1;  // skip past this char
        }
    }
    co_yield String(data, start, pos - start);
}

}  // namespace cxx

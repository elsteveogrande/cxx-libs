#pragma once

#include <cstdint>
#include <string>
#include <utility>

inline constexpr int64_t ce_strlen(char const *data) {
    auto ret = std::string::traits_type::length(data);
    if (ret <= 0x7fffffffffffffffL) { return int64_t(ret); }
    std::unreachable();
}

template <typename D>
inline constexpr void ce_strncpy(D *dest, D const *src, size_t size) {
    size_t i = 0uz;
    for (; i < size; i++) { dest[i] = (D &) src[i]; }
    dest[i] = 0;
}

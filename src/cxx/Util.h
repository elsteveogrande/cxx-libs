#pragma once
static_assert(__cplusplus >= 202300L, "cxx-libs requires C++23");
// (c) 2024 Steve O'Brien -- MIT License

#include <cstddef>
#include <cstdint>
#include <string>
#include <utility>

namespace cxx {

// Constexpr-friendly versions of functions

inline consteval int64_t cev_strlen(char const* data) {
    auto ret = std::string::traits_type::length(data);
    if (ret <= 0x7fffffffffffffffL) { return int64_t(ret); }
    std::unreachable();
}

inline constexpr int64_t ce_strlen(char const* data) {
    if consteval { return cev_strlen(data); }
    auto ret = std::string::traits_type::length(data);
    if (ret <= 0x7fffffffffffffffL) { return int64_t(ret); }
    std::unreachable();
}

template <typename D>
inline consteval void cev_memcpy(D* dest, D const* src, size_t size) {
    size_t i = 0uz;
    for (; i < size; i++) {
        dest[i] = (D&) src[i];
    }
    dest[i] = 0;
}

template <typename D>
inline constexpr void ce_memcpy(D* dest, D const* src, size_t size) {
    if consteval { return cev_memcpy(dest, src, size); }
    size_t i = 0uz;
    for (; i < size; i++) {
        dest[i] = (D&) src[i];
    }
    dest[i] = 0;
}

template <typename D>
inline consteval void cev_memset(D* dest, D byte, size_t size) {
    size_t i = 0uz;
    for (; i < size; i++) {
        dest[i] = byte;
    }
}

template <typename D>
inline constexpr void ce_memset(D* dest, D byte, size_t size) {
    if consteval { return cev_memset(dest, byte, size); }
    size_t i = 0uz;
    for (; i < size; i++) {
        dest[i] = byte;
    }
}

}  // namespace cxx

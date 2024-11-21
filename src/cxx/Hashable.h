#pragma once

#include <functional>

namespace cxx {
template <typename T>
concept Hashable = requires(T x) { std::hash(x); };
}  // namespace cxx

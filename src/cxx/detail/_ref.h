#pragma once
static_assert(__cplusplus >= 202300L, "cxx-libs requires C++23");
// (c) 2024 Steve O'Brien -- MIT License

#include <atomic>
#include <cstdint>

namespace cxx {

template <typename R>
class Ref;

namespace detail {

class RefCountedBase {
private:
    std::atomic_int64_t mutable rc_;

public:
    virtual ~RefCountedBase() = default;
    inline void inc() const { ++rc_; }
    inline bool dec() const { return --rc_ == -1; }
    int64_t count() const { return rc_; }
};

}  // namespace detail
}  // namespace cxx

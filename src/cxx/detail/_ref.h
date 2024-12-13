#pragma once
static_assert(__cplusplus >= 202300L, "cxx-libs requires C++23");
// (c) 2024 Steve O'Brien -- MIT License

#include <atomic>
#include <cstdint>

namespace cxx {

template <typename R>
class Ref;

namespace detail {

class RefCount final {
private:
    friend class RefCountedBase;

    /** zero-constructible */
    int64_t mutable refs_ {0};

    /** casted as an atomic; safe, because it's correctly aligned */
    std::atomic_int64_t& refs() const { return (std::atomic_int64_t&) refs_; }

public:
    /** Increment ref count */
    inline void inc() const { ++refs(); }

    /** Decrement ref count, and return whether it has now dropped to -1 */
    inline bool dec() const { return (--refs() == -1); }

    operator bool() const { return refs_ >= 0; }
};
static_assert(sizeof(RefCount) == 8);
static_assert(alignof(RefCount) == 8);

struct RefBase {};

class RefCountedBase {
private:
    friend struct RefBase;
    RefCount rc;

public:
    virtual ~RefCountedBase() = default;
    inline void inc() const { rc.inc(); }
    inline bool dec() const { return rc.dec(); }
    int64_t count() const { return rc.refs_; }
};

}  // namespace detail
}  // namespace cxx

#pragma once

#include <atomic>
#include <cassert>
#include <cstdint>

namespace cxx {

namespace internal {

struct RefCount {
    [[gnu::aligned(8)]] uint64_t mutable refs {0};

    inline std::atomic<uint64_t>& atomicRefs() const {
        return *(std::atomic<uint64_t>*) (&(this->refs));
    }

    void inc() const {
        auto& refs = atomicRefs();
        auto old = refs.load(std::memory_order_relaxed);
        while (!refs.compare_exchange_weak(old,
                                           old + 1,
                                           std::memory_order_release,
                                           std::memory_order_relaxed)) {}
    }

    bool dec() const {
        auto& refs = atomicRefs();
        auto old = refs.load(std::memory_order_relaxed);
        assert(old);
        while (!refs.compare_exchange_weak(old,
                                           old - 1,
                                           std::memory_order_release,
                                           std::memory_order_relaxed)) {}
        return old == 1;  // was 1, and now dropped to 0
    }

    void operator=(uint64_t newVal) { refs = newVal; }

    operator bool() const {
        return bool(atomicRefs().load(std::memory_order_relaxed));
    }
};
static_assert(sizeof(RefCount) == 8);

}  // namespace internal

}  // namespace cxx

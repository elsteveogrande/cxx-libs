#include "cxx/Generator.h"

#include <atomic>
#include <cassert>
#include <cxx/Ref.h>
#include <cxx/String.h>
#include <functional>
#include <iostream>
#include <memory>
#include <ostream>
#include <ranges>
#include <string>

namespace {

cxx::Generator<int> foo() {
    co_yield 1;
    co_yield 2;
}

void testCreateGenAndIter() {
    auto gen = foo();
    auto end = gen.end();
    int x;

    auto it = gen.begin();
    assert(it != end);
    x = *it;
    std::cerr << x << std::endl;
    assert(x == 1);

    ++it;
    assert(it != end);
    x = *it;
    std::cerr << x << std::endl;
    assert(x == 2);

    ++it;
    assert(it == end);
}

void testRangeConformance() {
    using G = cxx::Generator<int>;
    static_assert(std::ranges::range<G>);
    static_assert(std::ranges::view<G>);

    G gen = foo();
    auto begin = std::ranges::begin(gen);
    auto end = std::ranges::end(gen);
    (void) (begin == end);
    (void) (begin != end);
    (void) (end == begin);
    (void) (end != begin);
    int z = *begin;
}

void testGenMap() {
    std::function<int(int)> f = [](int x) { return x * 3; };

    auto gen = foo() | std::views::transform(f);
    auto end = gen.end();
    int x;

    auto it = gen.begin();
    assert(it != end);
    x = *it;
    std::cerr << x << std::endl;
    assert(x == 3);

    ++it;
    assert(it != end);
    x = *it;
    std::cerr << x << std::endl;
    assert(x == 6);

    ++it;
    assert(it == end);
}

void testGeneratableOrNot() {
    // Trivial stuff
    static_assert(cxx::Generatable<int>);
    static_assert(cxx::Generatable<bool>);
    static_assert(cxx::Generatable<float>);

    // Non-trivial stuff
    static_assert(cxx::Generatable<std::string>);
    static_assert(cxx::Generatable<cxx::String>);
    struct SomethingCopyable {};
    static_assert(cxx::Generatable<SomethingCopyable>);

    // Non-copyable stuff...
    struct NotCopyable {
        NotCopyable(NotCopyable const&) = delete;
        NotCopyable& operator=(NotCopyable const&) = delete;
    };
    static_assert(!cxx::Generatable<NotCopyable>);

    // Example error message if you do try `Generatable<NotCopyable>`:
    //   ... Because 'NotCopyable' does not satisfy 'Generatable'
    //   ... Because 'NotCopyable' does not satisfy 'semiregular'
    //   ... Because 'NotCopyable' does not satisfy 'copyable'
    //   ... Because 'NotCopyable' does not satisfy 'copy_constructible'
    //   ... Because 'NotCopyable' does not satisfy 'move_constructible'
    //   ... Because 'constructible_from<NotCopyable, NotCopyable>' evaluated to false
    //   ... Because 'is_constructible_v<NotCopyable, NotCopyable>' evaluated to false
    //   ... And 'std::is_copy_assignable_v<NotCopyable>' evaluated to false
    //   ... And 'std::is_base_of_v<RefBase, NotCopyable>' evaluated to false

    // More non-copyable stuff...
    static_assert(!cxx::Generatable<std::atomic_uint64_t>);

    // But when wrapped with copyable things like ptrs / Ref, no problem
    static_assert(cxx::Generatable<std::atomic_uint64_t*>);
    static_assert(cxx::Generatable<cxx::Ref<std::atomic_uint64_t>>);
    static_assert(cxx::Generatable<std::shared_ptr<std::atomic_uint64_t>>);
}

}  // namespace

int main() {
    testCreateGenAndIter();
    testRangeConformance();
    testGeneratableOrNot();
    testGenMap();
    return 0;
}

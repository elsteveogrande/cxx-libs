// Only include the header being tested; this single standalone include
// should work without needing other headers.
#include "cxx/Generator.h"

#include <cassert>
#include <functional>
#include <iostream>
#include <ostream>
#include <ranges>

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

}  // namespace

int main() {
    testCreateGenAndIter();
    testRangeConformance();
    testGenMap();
    return 0;
}

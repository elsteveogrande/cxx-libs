#include "cxx/Generator.h"
#include "cxx/Ref.h"
#include "cxx/test/Test.h"

#include <cassert>
#include <cxx/Exception.h>
#include <cxx/String.h>
#include <functional>
#include <iostream>
#include <ostream>
#include <ranges>
using cxx::test::Test;
int main(int, char**) { return cxx::test::run(); }

cxx::Generator<int> foo() {
    co_yield 1;
    co_yield 2;
}

Test createGenAndIter([] {
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
});

Test rangeConformance([] {
    using G = cxx::Generator<int>;
    static_assert(std::ranges::range<G>);

    G gen = foo();
    auto begin = std::ranges::begin(gen);
    auto end = std::ranges::end(gen);
    (void) (begin == end);
    (void) (begin != end);
    (void) (end == begin);
    (void) (end != begin);
    int z = *begin;
});

Test genTransform([] {
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
});

struct Foo {
    unsigned x;
    ~Foo() { printf("~Foo\n"); }
};

struct Bar {
    cxx::Ref<Foo> f;
};

cxx::Generator<Bar> genBars() {
    co_yield {cxx::Ref<Foo>::make(111)};
    co_yield {cxx::Ref<Foo>::make(222)};
}

Test noLeaksViaYield([] {
    for (auto b : genBars()) { (void) b; }
    // LSAN build will trigger an error if any leaks
});

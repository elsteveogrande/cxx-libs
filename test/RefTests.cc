#include "cxx/Generator.h"
#include "cxx/Ref.h"

#include <cassert>
#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

unsigned ctorCount {0};
unsigned dtorCount {0};

namespace {

struct Foo final {
    int a {42};
    double b {59.25};
    std::string c {"hello"};

    ~Foo() { ++dtorCount; }
    Foo() { ++ctorCount; }
    Foo(int a, double b, std::string c) : a(a), b(b), c(std::move(c)) { ++ctorCount; }
};

}  // namespace

int main() {
    // Number of times a `Foo` object was constructed or destructed
    assert(ctorCount == 0);
    assert(dtorCount == 0);
    auto noLeaks = [&]() { return dtorCount == ctorCount; };

    ctorCount = dtorCount = 0;
    std::cerr << "basic: make, no args" << std::endl;
    {
        auto ref = cxx::Ref<Foo>::make();
        assert(ref->a == 42);
        assert(ref->b == 59.25);
        assert(ref->c == "hello");
        assert(ctorCount == 1);
        assert(dtorCount == 0);
    }
    assert(noLeaks());

    ctorCount = dtorCount = 0;
    std::cerr << "basic/make with args" << std::endl;
    {
        auto ref = cxx::Ref<Foo>::make(1, 2, "test3");
        assert(ref->a == 1);
        assert(ref->b == 2);
        assert(ref->c == "test3");
        assert(ctorCount == 1);
        assert(dtorCount == 0);
    }
    assert(noLeaks());

    ctorCount = dtorCount = 0;
    std::cerr << "copy ctor" << std::endl;
    {
        auto ref = cxx::Ref<Foo>::make();
        std::cerr << "copy ctor: before: refs=" << ref._refs() << std::endl;
        assert(ref._refs() == 1);
        assert(ctorCount == 1);
        assert(dtorCount == 0);
        auto ref2(ref);
        std::cerr << "copy ctor: after:  refs=" << ref._refs() << std::endl;
        assert(ref._refs() == 2);
        assert(ctorCount == 1);
        assert(dtorCount == 0);
    }
    assert(noLeaks());

    ctorCount = dtorCount = 0;
    std::cerr << "copy assign" << std::endl;
    {
        auto ref = cxx::Ref<Foo>::make();
        assert(ref._refs() == 1);
        assert(ctorCount == 1);
        assert(dtorCount == 0);
        auto ref2 = ref;
        assert(ref._refs() == 2);
        assert(ctorCount == 1);
        assert(dtorCount == 0);
    }
    assert(noLeaks());

    ctorCount = dtorCount = 0;
    std::cerr << "move ctor" << std::endl;
    {
        auto ref = cxx::Ref<Foo>::make();
        assert(ref._refs() == 1);
        assert(ctorCount == 1);
        assert(dtorCount == 0);
        auto ref2(std::move(ref));
        assert(ref2._refs() == 1);
        assert(ctorCount == 1);
        assert(dtorCount == 0);
    }
    assert(noLeaks());

    ctorCount = dtorCount = 0;
    std::cerr << "move assign" << std::endl;
    {
        auto ref = cxx::Ref<Foo>::make();
        assert(ref._refs() == 1);
        assert(ctorCount == 1);
        assert(dtorCount == 0);
        auto ref2 = std::move(ref);
        assert(ref2._refs() == 1);
        assert(ctorCount == 1);
        assert(dtorCount == 0);
    }
    assert(noLeaks());

    struct Bar {
        cxx::Ref<Foo> foo;
    };

    ctorCount = dtorCount = 0;
    std::cerr << "ref inside a struct" << std::endl;
    {
        Bar bar {cxx::Ref<Foo>::make()};
        assert(bar.foo._refs() == 1);
        assert(ctorCount == 1);
        assert(dtorCount == 0);
        auto bar2(bar);
        assert(bar.foo._refs() == 2);
        assert(ctorCount == 1);
        assert(dtorCount == 0);
    }
    assert(noLeaks());

    ctorCount = dtorCount = 0;
    std::cerr << "ref inside a ref" << std::endl;
    {
        auto bar = cxx::Ref<Bar>::make(cxx::Ref<Foo>::make());
        assert(bar->foo._refs() == 1);
        assert(bar._refs() == 1);
        assert(ctorCount == 1);
        assert(dtorCount == 0);
        auto bar2(bar);
        assert(bar->foo._refs() == 1);
        assert(bar._refs() == 2);
        assert(ctorCount == 1);
        assert(dtorCount == 0);
    }
    assert(noLeaks());

    ctorCount = dtorCount = 0;
    std::cerr << "ref inside a vector" << std::endl;
    {
        auto foo = cxx::Ref<Foo>::make();
        assert(foo._refs() == 1);
        std::vector<cxx::Ref<Foo>> vec;
        for (int i = 0; i < 10; i++) { vec.push_back(foo); }
        assert(foo._refs() == 11);
        assert(ctorCount == 1);
        assert(dtorCount == 0);
    }
    assert(noLeaks());

    ctorCount = dtorCount = 0;
    std::cerr << "ref inside a vector ref" << std::endl;
    {
        auto bar = cxx::Ref<Bar>::make(cxx::Ref<Foo>::make());
        assert(bar->foo._refs() == 1);
        assert(bar._refs() == 1);
        std::vector<cxx::Ref<Bar>> vec;
        for (int i = 0; i < 10; i++) { vec.push_back(bar); }
        assert(bar->foo._refs() == 1);
        assert(bar._refs() == 11);
        assert(ctorCount == 1);
        assert(dtorCount == 0);
    }
    assert(noLeaks());

    ctorCount = dtorCount = 0;
    std::cerr << "ref from generator" << std::endl;
    {
        auto gen = [&]() -> cxx::Generator<cxx::Ref<Foo>> {
            co_yield cxx::Ref<Foo>::make();
            co_yield cxx::Ref<Foo>::make();
        };
        for (auto f : gen()) {}
    }
    assert(noLeaks());

    // TODO tests which verify thread-safety

    return 0;
}

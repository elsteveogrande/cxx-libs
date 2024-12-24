// Only include the header being tested; this single standalone include
// should work without needing other headers.
#include "cxx/Ref.h"

#include <cassert>
#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <string>
#include <utility>

unsigned ctorCount {0};
unsigned dtorCount {0};

namespace {

struct Foo final {
    int a {42};
    double b {59.25};
    std::string c {"hello"};

    ~Foo() { ++dtorCount; }
    Foo() {
        std::cerr << "Foo ctor" << std::endl;
        ++ctorCount;
    }
    Foo(int a, double b, std::string c) : a(a), b(b), c(std::move(c)) { ++ctorCount; }
};

}  // namespace

int main() {
    // Number of times a `Foo` object was constructed or destructed
    assert(ctorCount == 0);
    assert(dtorCount == 0);
    auto noLeaks = [&]() { return dtorCount == ctorCount; };

    std::cerr << "basic: make, no args" << std::endl;  // One day I'll get around to StackTrace ...
    // Basic construction: `make` (no args)
    {
        auto ref = cxx::ref<Foo>();
        assert(ref->a == 42);
        assert(ref->b == 59.25);
        assert(ref->c == "hello");
        assert(ctorCount == 1);
        assert(dtorCount == 0);
    }
    assert(noLeaks());

    std::cerr << "basic/make with args" << std::endl;
    // Basic construction: `make` (with args)
    {
        auto ref = cxx::ref<Foo>(1, 2, "test3");
        assert(ref->a == 1);
        assert(ref->b == 2);
        assert(ref->c == "test3");
        assert(ctorCount == 2);
        assert(dtorCount == 1);
    }
    assert(noLeaks());

    std::cerr << "copy ctor" << std::endl;
    // Copy-construct
    {
        auto ref = cxx::ref<Foo>();
        assert(ctorCount == 3);
        assert(dtorCount == 2);
        auto ref2(ref);
        assert(ctorCount == 3);
        assert(dtorCount == 2);
    }
    assert(noLeaks());

    std::cerr << "move ctor" << std::endl;
    // Move-construct
    {
        auto ref = cxx::ref<Foo>();
        assert(ctorCount == 4);
        assert(dtorCount == 3);
        auto ref2(std::move(ref));
        assert(ctorCount == 4);
        assert(dtorCount == 3);
    }
    assert(noLeaks());

    return 0;
}

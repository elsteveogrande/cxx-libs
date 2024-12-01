// Only include the header being tested; this single standalone include
// should work without needing other headers.
#include "cxx/Ref.h"

#include <cassert>
#include <cstddef>
#include <cstdlib>
#include <string>
#include <utility>

unsigned ctorCount {0};
unsigned dtorCount {0};

namespace {

struct Foo : cxx::RefCounted<Foo> {
    int a {42};
    double b {59.25};
    std::string c {"hello"};

    ~Foo() { ++dtorCount; }

    Foo() : cxx::RefCounted<Foo>() { ++ctorCount; }

    Foo(int a, double b, std::string c) : a(a), b(b), c(std::move(c)) {
        ++ctorCount;
    }
};

}  // namespace

int main() {
    // Number of times a `Foo` object was constructed or destructed
    assert(ctorCount == 0);
    assert(dtorCount == 0);
    auto noLeaks = [&]() { return dtorCount == ctorCount; };

    // Basic construction: `make` (no args)
    {
        auto ref = cxx::make<Foo>();
        assert(ref->a == 42);
        assert(ref->b == 59.25);
        assert(ref->c == "hello");
        assert(ctorCount == 1);
        assert(dtorCount == 0);
    }
    assert(noLeaks());

    // Basic construction: `make` (with args)
    {
        auto ref = cxx::make<Foo>(1, 2, "test3");
        assert(ref->a == 1);
        assert(ref->b == 2);
        assert(ref->c == "test3");
        assert(ctorCount == 2);
        assert(dtorCount == 1);
    }
    assert(noLeaks());

    // Copy-construct
    {
        auto ref = cxx::make<Foo>();
        assert(ctorCount == 3);
        assert(dtorCount == 2);
        auto ref2(ref);
        assert(ctorCount == 3);
        assert(dtorCount == 2);
    }
    assert(noLeaks());

    // Move-construct
    {
        auto ref = cxx::make<Foo>();
        assert(ctorCount == 4);
        assert(dtorCount == 3);
        auto ref2(std::move(ref));
        assert(ctorCount == 4);
        assert(dtorCount == 3);
    }
    assert(noLeaks());

    return 0;
}

#include "cxx/Generator.h"
#include "cxx/Ref.h"
#include "cxx/test/Test.h"

#include <atomic>
#include <cassert>
#include <cstddef>
#include <cstdlib>
#include <functional>
#include <string>
#include <thread>
#include <utility>
#include <vector>

int main(int, char**) { return cxx::test::run(); }

// Number of times a `Foo` object was constructed or destructed
unsigned ctorCount {0};
unsigned dtorCount {0};

// Wrap internal `Test` with this one which has before/after ops
struct Test : cxx::test::Test {
    Test(std::function<void()> func)
            : cxx::test::Test([=] {
                ctorCount = dtorCount = 0;
                func();
                assert(ctorCount == dtorCount);
            }) {}
};

struct Foo final {
    int a {42};
    double b {59.25};
    std::string c {"hello"};
    ~Foo() { ++dtorCount; }
    Foo() { ++ctorCount; }
    Foo(int a, double b, std::string c) : a(a), b(b), c(std::move(c)) { ++ctorCount; }
};

Test makeWithNoArgs([] {
    auto ref = cxx::Ref<Foo>::make();
    assert(ref->a == 42);
    assert(ref->b == 59.25);
    assert(ref->c == "hello");
    assert(ctorCount == 1);
    assert(dtorCount == 0);
});

Test makeWithArgs([] {
    auto ref = cxx::Ref<Foo>::make(1, 2, "test3");
    assert(ref->a == 1);
    assert(ref->b == 2);
    assert(ref->c == "test3");
    assert(ctorCount == 1);
    assert(dtorCount == 0);
});

Test copyConstruct([] {
    auto ref = cxx::Ref<Foo>::make();
    assert(ref._refs() == 1);
    assert(ctorCount == 1);
    assert(dtorCount == 0);
    auto ref2(ref);
    assert(ref._refs() == 2);
    assert(ctorCount == 1);
    assert(dtorCount == 0);
});

Test copyAssign([] {
    auto ref = cxx::Ref<Foo>::make();
    assert(ref._refs() == 1);
    assert(ctorCount == 1);
    assert(dtorCount == 0);
    auto ref2 = ref;
    assert(ref._refs() == 2);
    assert(ctorCount == 1);
    assert(dtorCount == 0);
});

Test moveConstruct([] {
    auto ref = cxx::Ref<Foo>::make();
    assert(ref._refs() == 1);
    assert(ctorCount == 1);
    assert(dtorCount == 0);
    auto ref2(std::move(ref));
    assert(ref2._refs() == 1);
    assert(ctorCount == 1);
    assert(dtorCount == 0);
});

Test moveAssign([] {
    auto ref = cxx::Ref<Foo>::make();
    assert(ref._refs() == 1);
    assert(ctorCount == 1);
    assert(dtorCount == 0);
    auto ref2 = std::move(ref);
    assert(ref2._refs() == 1);
    assert(ctorCount == 1);
    assert(dtorCount == 0);
});

struct Bar {
    cxx::Ref<Foo> foo;
};

Test refInsideStruct([] {
    Bar bar {cxx::Ref<Foo>::make()};
    assert(bar.foo._refs() == 1);
    assert(ctorCount == 1);
    assert(dtorCount == 0);
    auto bar2(bar);
    assert(bar.foo._refs() == 2);
    assert(ctorCount == 1);
    assert(dtorCount == 0);
});

Test refInsideRef([] {
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
});

Test refInsideVector([] {
    auto foo = cxx::Ref<Foo>::make();
    assert(foo._refs() == 1);
    std::vector<cxx::Ref<Foo>> vec;
    for (int i = 0; i < 10; i++) { vec.push_back(foo); }
    assert(foo._refs() == 11);
    assert(ctorCount == 1);
    assert(dtorCount == 0);
});

Test refInsideVectorRef([] {
    auto bar = cxx::Ref<Bar>::make(cxx::Ref<Foo>::make());
    assert(bar->foo._refs() == 1);
    assert(bar._refs() == 1);
    std::vector<cxx::Ref<Bar>> vec;
    for (int i = 0; i < 10; i++) { vec.push_back(bar); }
    assert(bar->foo._refs() == 1);
    assert(bar._refs() == 11);
    assert(ctorCount == 1);
    assert(dtorCount == 0);
});

Test refFromGenerator([] {
    auto gen = [&]() -> cxx::Generator<cxx::Ref<Foo>> {
        co_yield cxx::Ref<Foo>::make();
        co_yield cxx::Ref<Foo>::make();
    };
    for (auto f : gen()) {}
});

std::atomic_bool threadsReady {false};
std::atomic_bool thingDeleted {false};

struct Thing {
    ~Thing() { thingDeleted = true; }
};

void threadFunc(cxx::Ref<Thing> const& thing) {
    while (!threadsReady) {}  // busy-wait
    for (int i = 0; i < 1000000; i++) {
        assert(!thingDeleted);
        auto ref = thing;  // take a copy of this Ref
        (void) ref.get();  // pretend to do stuff with this object
        // at this point the copied Ref goes out of scope
    }
}

Test refSharedByThreads([] {
    auto thing = cxx::Ref<Thing>::make();
    std::thread t1([thing] { threadFunc(thing); });
    std::thread t2([thing] { threadFunc(thing); });
    std::thread t3([thing] { threadFunc(thing); });
    std::thread t4([thing] { threadFunc(thing); });
    threadsReady = true;
    t1.join();
    t2.join();
    t3.join();
    t4.join();
    assert(!thingDeleted);
    thing.clear();
    assert(thingDeleted);
});

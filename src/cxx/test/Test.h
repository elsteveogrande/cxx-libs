#pragma once
static_assert(__cplusplus >= 202300L, "cxx-libs requires C++23");
// (c) 2024 Steve O'Brien -- MIT License

#include <cxx/Exception.h>
#include <functional>
#include <list>
#include <utility>

namespace cxx::test {

struct AssertFailed : Exception {};

struct Test;

struct Tests {
    using F = std::function<void()>;

    static Tests& get() {
        static Tests ret;
        return ret;
    }

    std::list<Test*> tests;

    int run(Test& test);
    int run();
};

struct Test {
    std::function<void()> func_;

    Test(std::function<void()> func) : func_(std::move(func)) {
        Tests::get().tests.push_back(this);
    }
};

int Tests::run(Test& test) {
    // try { ... } catch (AssertFailed&) {}
    printf("%p\n", &test);
    test.func_();
    return 0;
}

int Tests::run() {
    int ret = 0;
    auto it = tests.begin();
    auto end = tests.end();
    while (it != end) {
        ret |= run(**it);
        it = tests.erase(it);
    }
    return ret;
}

int run() { return Tests::get().run(); }

}  // namespace cxx::test

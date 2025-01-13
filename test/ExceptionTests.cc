#include "cxx/Exception.h"
#include "cxx/test/Test.h"

#include <cassert>
#include <string>

using cxx::test::Test;
int main(int, char**) { return cxx::test::run(); }

struct TestException : cxx::Exception<TestException> {};

void func2() { throw TestException() << "Test" << "Message"; }
void func1() { func2(); }

Test testSimpleThrowCatch([] {
    try {
        func1();
    } catch (TestException const& e) {
        //
        assert(std::string("TestMessage") == e.what());
    }
});

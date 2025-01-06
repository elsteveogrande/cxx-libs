#include <cassert>
#include <cxx/Exception.h>
#include <string>

namespace {
struct TestException : cxx::Exception {
    TestException() : cxx::Exception("test") {}
};
}  // namespace

namespace test {
void func2() { throw TestException(); }
void func1() { func2(); }
}  // namespace test

int main() {
    try {
        test::func1();
    } catch (TestException const& e) {
        assert(std::string("test") == e.what());
        return 0;
    }
    return 1;
}

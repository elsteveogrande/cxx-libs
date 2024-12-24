// Only include the header being tested; this single standalone include
// should work without needing other headers.
#include "cxx/Exception.h"

#include <cassert>
#include <string>

namespace {
struct TestException : cxx::Exception {
    TestException() : cxx::Exception("test") {}
};
}  // namespace

int main() {
    try {
        throw TestException();
    } catch (TestException const& e) {
        assert(std::string("test") == e.what());
        return 0;
    }
    return 1;
}

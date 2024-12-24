// Only include the header being tested; this single standalone include
// should work without needing other headers.
#include "cxx/StackTrace.h"

#include <cassert>

int main() {
    cxx::StackTrace s;
    // assert(std::string(s.frame->filename) == "test/StackTraceTests.cc");
    return 0;
}

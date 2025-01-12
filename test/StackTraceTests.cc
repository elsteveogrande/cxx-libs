#include "cxx/test/Test.h"

#include <cassert>
#include <cxx/StackTrace.h>
#include <iostream>

using cxx::test::Test;
int main(int, char**) { return cxx::test::run(); }

Test simpleDump([] {
    cxx::StackResolver sr;
    cxx::StackTrace s;
    s.resolve(sr);
    s.dump(std::cout);
    return 0;
});

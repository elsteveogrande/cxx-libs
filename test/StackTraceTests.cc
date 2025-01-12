#include "cxx/StackTrace.h"
#include "cxx/String.h"
#include "cxx/test/Test.h"

#include <cassert>
#include <list>
#include <sstream>

using cxx::test::Test;
int main(int, char**) { return cxx::test::run(); }

Test simpleDump([] {
    cxx::StackResolver sr;
    cxx::StackTrace s;
    s.resolve(sr);
    std::stringstream ss;
    s.dump(ss);
    auto lines = cxx::String(ss.str()).split('\n').to<std::list<cxx::String>>();
    assert(lines.size() > 1);
});

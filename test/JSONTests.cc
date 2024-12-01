// Only include the header being tested; this single standalone include
// should work without needing other headers.
#include "cxx/JSON.h"
#include "cxx/String.h"

#include <cassert>
#include <iostream>
#include <sstream>
#include <vector>

template <typename T> void expectJSONResult(cxx::String str, T expr) {
    std::cerr << "expect: " << str << std::endl;
    std::stringstream ss;
    cxx::JSON(expr).write(ss);
    std::cerr << "actual: " << ss.str() << std::endl;
    assert(str == cxx::String(ss.str()));
}

int main() {
    expectJSONResult("null", nullptr);

    expectJSONResult("true", true);
    expectJSONResult("false", false);

    expectJSONResult("1", 1);
    expectJSONResult("0", 0);
    expectJSONResult("-1", -1);
    expectJSONResult("1.5", 1.5);
    expectJSONResult("-3.125", -3.125);

    expectJSONResult("\"hello world\"", "hello world");

    expectJSONResult("[1,2,3]", std::vector<int> {1, 2, 3});

    // expectJSONResult(
    //         R"(["hello","world"])",
    //         cxx::String("hello world a b c d e f g").split(' '));

    return 0;
}

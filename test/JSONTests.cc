#include "cxx/Generator.h"
#include "cxx/JSON.h"
#include "cxx/String.h"

#include <array>
#include <cassert>
#include <cstddef>
#include <cxx/Ref.h>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

template <typename T>
void expectJSONResult(cxx::String str, T expr) {
    std::cerr << "expect: " << str << std::endl;
    std::stringstream ss;
    cxx::JSON(expr).write(ss);
    ss.flush();
    std::cerr << "actual: " << ss.str() << std::endl;
    assert(str == cxx::String(ss.str()));
}

int main() {
    // Writing JSON

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

    expectJSONResult(
            R"(["hello","world","a","b","c","d","e"])", cxx::String("hello world a b c d e").split(' '));

    expectJSONResult(R"({"x":"y"})", std::map<cxx::String, cxx::String> {{"x", "y"}});
    expectJSONResult(R"({"x":"y"})", std::map<std::string, std::string> {{"x", "y"}});
    expectJSONResult(R"({"x":"y"})", std::map<cxx::String, cxx::String> {{"x", "y"}});

    struct JSONableStruct {
        cxx::Generator<cxx::JSON::ObjectProp> genJSONProps() const {
            co_yield {.key = "hello", .val = cxx::make<cxx::JSON>(std::array<bool, 2> {false, true})};
            co_yield {.key = "world", .val = cxx::make<cxx::JSON>(nullptr)};
            co_yield {.key = "array", .val = cxx::make<cxx::JSON>(std::vector<double> {-1.5, 2})};
            co_yield {.key = "obj",
                      .val = cxx::make<cxx::JSON>(std::map<cxx::String, std::vector<cxx::String>> {
                              {"x", {"y", "z"}}})};
        }
    };

    auto const json = R"({"hello":[false,true],"world":null,"array":[-1.5,2],"obj":{"x":["y","z"]}})";
    expectJSONResult(json, JSONableStruct().genJSONProps());

    // // Parsing JSON

    // assert(*cxx::JSON::parse("null") == cxx::JSONNull());

    return 0;
}

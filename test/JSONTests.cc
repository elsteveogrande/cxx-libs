#include "cxx/Generator.h"
#include "cxx/JSON.h"
#include "cxx/String.h"

#include <cassert>
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
    expectJSONResult("null", nullptr);

    expectJSONResult("true", true);
    expectJSONResult("false", false);

    expectJSONResult("1", 1);
    expectJSONResult("0", 0);
    expectJSONResult("-1", -1);
    expectJSONResult("1.5", 1.5);
    expectJSONResult("-3.125", -3.125);

    expectJSONResult("\"hello world\"", "hello world");

    struct Stringish final {
        char const c_;
        explicit Stringish(char c) : c_(c) {}
        operator cxx::String() const { return {c_}; }
    };

    expectJSONResult("[1,2,3]", std::vector<int> {1, 2, 3});

    expectJSONResult(
            R"(["hello","world","a","b","c","d","e"])", cxx::String("hello world a b c d e").split(' '));

    expectJSONResult(R"({"x":"y"})", std::map<cxx::String, cxx::String> {{"x", "y"}});
    expectJSONResult(R"({"x":"y"})", std::map<std::string, std::string> {{"x", "y"}});
    expectJSONResult(R"({"x":"y"})", std::map<cxx::String, cxx::String> {{"x", "y"}});

    struct JSONableStruct {
        cxx::Generator<cxx::JSONProp> genJSONProps() const {
            co_yield {"hello", true};
            co_yield {"world", nullptr};
            co_yield {"array", std::vector<int> {1, 2}};
            co_yield {"obj", std::map<cxx::String, cxx::String> {{"x", "y"}}};
        }
    };

    expectJSONResult(R"({"hello":true,"world":null,"array":[1,2],"obj":{"x":"y"}})", JSONableStruct());

    return 0;
}

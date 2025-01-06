#include "cxx/Generator.h"
#include "cxx/JSON.h"
#include "cxx/String.h"

#include <array>
#include <cassert>
#include <cstddef>
#include <iostream>
#include <list>
#include <map>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

template <typename T>
void expectJSONResult(cxx::String str, T&& expr) {
    std::cerr << "expect: " << str << std::endl;
    std::stringstream ss;
    cxx::JSON j(std::forward<T>(expr));
    j.write(ss);
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

    std::optional<double> maybe {3.25};
    expectJSONResult("3.25", maybe);
    maybe.reset();
    expectJSONResult("null", maybe);

    expectJSONResult("\"hello world\"", "hello world");

    expectJSONResult("[66]", std::array<int, 1> {66});
    expectJSONResult("[1,2,3]", std::array<int, 3> {1, 2, 3});
    expectJSONResult("[1,2,3]", std::vector<int> {1, 2, 3});
    expectJSONResult("[1,2,3]", std::list<int> {1, 2, 3});

    expectJSONResult(
            R"(["hello","world","a","b","c","d","e"])", cxx::String("hello world a b c d e").split(' '));

    expectJSONResult(R"({"x":"y"})", std::map<cxx::String, cxx::String> {{"x", "y"}});
    expectJSONResult(R"({"x":"y"})", std::map<std::string, std::string> {{"x", "y"}});
    expectJSONResult(R"({"x":"y"})", std::map<cxx::String, cxx::String> {{"x", "y"}});

    struct JSONableStruct {
        using NumberArray = std::vector<double>;
        using Map = std::map<cxx::String, std::vector<cxx::String>>;
        cxx::Generator<cxx::ObjectProp> genJSONProps() const {
            // NOLINTBEGIN modernize-use-designated-initializers
            co_yield {"bools", std::array<bool, 2> {false, true}};
            co_yield {"nullable", nullptr};
            co_yield {"numberArray", NumberArray {-1.5, 2}};
            co_yield {"someObject", Map {{"x", {"y", "z"}}}};
            // NOLINTEND
        }
    };

    auto const json = R"({"bools":[false,true],"nullable":null,"numberArray":[-1.5,2],"someObject":{"x":["y","z"]}})";
    expectJSONResult(json, JSONableStruct().genJSONProps());

    // Parsing JSON

    // assert(cxx::JSON::parse("null") == cxx::JSON(nullptr));

    return 0;
}

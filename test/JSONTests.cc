#include "cxx/Generator.h"
#include "cxx/JSON.h"
#include "cxx/String.h"
#include "cxx/test/Test.h"

#include <array>
#include <cassert>
#include <cstddef>
#include <cstring>
#include <list>
#include <map>
#include <optional>
#include <string>
#include <vector>

using cxx::test::Test;
int main(int, char**) { return cxx::test::run(); }

template <typename T>
void expectJSONString(cxx::String str, T&& expr) {
    cxx::JSON j(std::forward<T>(expr));
    assert(str == j.str());
}

void expectParsedValue(cxx::JSON const& exp, cxx::String str) {
    auto result = cxx::JSON::parse(str);  // parse json string (returns an `Expected`)
    auto json = *result;                  // expect a value (otherwise throw the contained error)
    assert(json == exp);                  // assert expected `JSON` obj returned
    expectJSONString(str, json);          // write that to a string, expect equal to input
}

Test writingBasicTypes([] {
    expectJSONString("null", nullptr);
    expectJSONString("true", true);
    expectJSONString("false", false);
});

Test writingNumbers([] {
    expectJSONString("1", 1);
    expectJSONString("0", 0);
    expectJSONString("-1", -1);
    expectJSONString("1.5", 1.5);
    expectJSONString("-3.125", -3.125);
});

Test writingOptionals([] {
    std::optional<double> maybe {3.25};
    expectJSONString("3.25", maybe);
    maybe.reset();
    expectJSONString("null", maybe);
});

Test writingStrings([] { expectJSONString("\"hello world\"", "hello world"); });

Test writingArrays([] {
    expectJSONString("[]", std::vector<int>());
    expectJSONString("[1,2,3]", std::array<int, 3> {1, 2, 3});
    expectJSONString("[1,2,3]", std::vector<int> {1, 2, 3});
    expectJSONString("[1,2,3]", std::list<int> {1, 2, 3});

    expectJSONString(
            R"(["hello","world","a","b","c","d","e"])", cxx::String("hello world a b c d e").split(' '));
});

Test writingObjects([] {
    expectJSONString(R"({"x":"y"})", std::map<cxx::String, cxx::String> {{"x", "y"}});
    expectJSONString(R"({"x":"y"})", std::map<std::string, std::string> {{"x", "y"}});
    expectJSONString(R"({"x":"y"})", std::map<cxx::String, cxx::String> {{"x", "y"}});

    struct JSONableStruct {
        using NumberArray = std::vector<double>;
        using Map = std::map<cxx::String, std::vector<cxx::String>>;
        cxx::Generator<cxx::ObjectProp> genJSONProps() const {
            // NOLINTBEGIN modernize-use-designated-initializers
            co_yield {"bools", std::array<bool, 2> {false, true}};
            co_yield {"null1", nullptr};
            co_yield {"null2", nullptr};
            co_yield {"numberArray", NumberArray {-1.5, 2}};
            co_yield {"someObject", Map {{"x", {"y", "z"}}}};
            // NOLINTEND
        }
    };

    auto const json = R"({"bools":[false,true],"null1":null,"null2":null,"numberArray":[-1.5,2],"someObject":{"x":["y","z"]}})";
    expectJSONString(json, JSONableStruct().genJSONProps());
});

Test parsingBasicTypes([] {
    expectParsedValue(cxx::JSON(nullptr), "null");
    expectParsedValue(cxx::JSON(true), "true");
    expectParsedValue(cxx::JSON(false), "false");
});

Test parsingNumbers([] {
    expectParsedValue(cxx::JSON(1), "1");
    expectParsedValue(cxx::JSON(0), "0");
    expectParsedValue(cxx::JSON(-1), "-1");
    expectParsedValue(cxx::JSON(1.5), "1.5");
    expectParsedValue(cxx::JSON(-3.125), "-3.125");
});

Test parsingStrings([] {
    expectParsedValue(cxx::JSON(""), "\"\"");
    expectParsedValue(cxx::JSON("hello"), "\"hello\"");
});

Test parsingArrays([] {
    expectParsedValue(cxx::JSON(std::list<int> {}), "[]");
    expectParsedValue(cxx::JSON(std::list<int> {1}), "[1]");
    expectParsedValue(cxx::JSON(std::list<int> {1, 2}), "[1,2]");
});

Test parsingIntoJSONObjects([] {
    using Map = std::map<cxx::String, std::vector<cxx::String>>;
    expectParsedValue(cxx::JSON(Map {}), "{}");
    expectParsedValue(cxx::JSON(Map {{"x", {"y", "z"}}}), R"({"x":["y","z"]})");
});

// struct TestObject final {
//     std::vector<cxx::String> x;
// };

// Test parsingIntoRealObjects([] {
//     using Map = std::map<cxx::String, std::vector<cxx::String>>;
//     expectParsedValue(cxx::JSON(Map {}), "{}");
//     expectParsedValue(cxx::JSON(Map {{"x", {"y", "z"}}}), R"({"x":["y","z"]})");
// });

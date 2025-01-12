#include "cxx/test/Test.h"

#include <cassert>
#include <cstdlib>
#include <cstring>
#include <cxx/String.h>
#include <iomanip>
#include <ios>
#include <iostream>
#include <string>
#include <utility>

using cxx::test::Test;
int main(int, char**) { return cxx::test::run(); }

using cxx::String;

constexpr char const* kLongStringLiteral = "stringWhichHasThirtyCharacters";

namespace fancydump {

struct L {
    int const n;
    friend std::ostream& operator<<(std::ostream& os, L const& x) {
        os << std::setw(x.n) << std::left;
        return os;
    };
};

struct R {
    int const n;
    friend std::ostream& operator<<(std::ostream& os, R const& x) {
        os << std::setw(x.n) << std::right;
        return os;
    };
};

}  // namespace fancydump

Test constructEmptyCStringCE([] {
    constexpr String const s;
    assert(0 == s.size());
    assert(s.data());
    assert(0 == strcmp("", s.data()));
});

Test constructEmptyCString([] {
    String const s;
    assert(0 == s.size());
    assert(s.data());
    assert(0 == strcmp("", s.data()));
});

Test constructTinyCStringCE([] {
    constexpr String s = "AAAAA";
    assert(5 == s.size());
    assert(0 == strcmp("AAAAA", s.data()));
});

Test constructTinyCString([] {
    String s = "AAAAA";
    assert(5 == s.size());
    assert(0 == strcmp("AAAAA", s.data()));
});

Test constructCStringFromLiteralCE([] {
    constexpr String s(kLongStringLiteral);
    assert(30 == s.size());
    assert(kLongStringLiteral == s.data());
});

Test constructCStringFromLiteral([] {
    String s(kLongStringLiteral);
    assert(30 == s.size());
    assert(kLongStringLiteral != s.data());
    assert(0 == strcmp(kLongStringLiteral, s.data()));
});

Test constructCStringFromBuffer([] {
    char chars[31];
    memcpy(chars, "stringWhichHasThirtyCharacters", 30);
    chars[30] = 0;
    String s = chars;
    assert(30 == s.size());
    assert(chars != s.data());
    assert(0 == strcmp(kLongStringLiteral, s.data()));
});

Test constructCStringFromBufferAndReturn([] {
    auto func = [] {
        char chars[31];
        memcpy(chars, "stringWhichHasThirtyCharacters", 30);
        chars[30] = 0;
        String s = chars;
        return s;
    };
    auto const s = func();
    assert(30 == s.size());
    assert(0 == strcmp(kLongStringLiteral, s.data()));
});

Test copyConstructCE([] {
    static constexpr String s0 = kLongStringLiteral;
    constexpr String s(s0);
    assert(s.size() == s0.size());
    assert(s.data() == s0.data());
});

Test copyConstruct([] {
    String s0 = kLongStringLiteral;
    String s1(s0);
    assert(s1.size() == s0.size());
    assert(s1.data() == s0.data());
});

Test copyAssign([] {
    String s0;
    String s1 = kLongStringLiteral;
    assert(0 == s0.size());
    assert(0 == strcmp("", s0.data()));
    assert(30 == s1.size());
    assert(0 == strcmp(kLongStringLiteral, s1.data()));
    s0 = s1;
    assert(30 == s0.size());
    assert(0 == strcmp(kLongStringLiteral, s0.data()));
    assert(30 == s1.size());
    assert(0 == strcmp(kLongStringLiteral, s1.data()));
});

Test moveAssignCE([] {
    constexpr String s0 = kLongStringLiteral;
    assert(30 == s0.size());
    assert(0 == strcmp(kLongStringLiteral, s0.data()));
    String s1 = std::move(s0);
    // Doesn't work in constexpr context?
    // assert(0 == s0.size());
    // assert(0 == strcmp("", s0.data()));
    assert(30 == s1.size());
    assert(0 == strcmp(kLongStringLiteral, s1.data()));
});

Test moveConstruct([] {
    String s0 = kLongStringLiteral;
    assert(30 == s0.size());
    assert(0 == strcmp(kLongStringLiteral, s0.data()));
    String s1(std::move(s0));
    assert(0 == s0.size());
    assert(0 == strcmp("", s0.data()));
    assert(30 == s1.size());
    assert(0 == strcmp(kLongStringLiteral, s1.data()));
});

Test moveAssign([] {
    String s0 = "foo";
    String s1 = kLongStringLiteral;
    assert(3 == s0.size());
    assert(0 == strcmp("foo", s0.data()));
    assert(30 == s1.size());
    assert(0 == strcmp(kLongStringLiteral, s1.data()));
    s0 = std::move(s1);
    assert(30 == s0.size());
    assert(0 == strcmp(kLongStringLiteral, s0.data()));
    assert(0 == s1.size());
    assert(0 == strcmp("", s1.data()));
});

Test concatCE([] {
    constexpr String s1 = "foo";
    constexpr String s2 = kLongStringLiteral;
    assert(33 == (s1 + s2).size());
    assert('f' == (s1 + s2)[0]);
    assert('o' == (s1 + s2)[1]);
    assert('o' == (s1 + s2)[2]);
    assert('s' == (s1 + s2)[3]);
});

Test concat([] {
    String s1 = "foo";
    String s2 = kLongStringLiteral;
    assert(33 == (s1 + s2).size());
    assert('f' == (s1 + s2)[0]);
    assert('o' == (s1 + s2)[1]);
    assert('o' == (s1 + s2)[2]);
    assert('s' == (s1 + s2)[3]);
});

Test compare([] {
    String f = "foo";
    assert(f < "z");
    assert(f < String("z"));
    assert(f < std::string("z"));
    assert(f != "z");
    assert(f != String("z"));
    assert(f != std::string("z"));

    String b = "bar";
    assert(b < f);
    assert(b < String(f));
    assert(b < std::string(f));
    assert(b != f);
    assert(b != String(f));
    assert(b != std::string(f));

    String g = "foo";
    assert(g == f);
    assert(g == String(f));
    assert(g == std::string(f));
});

Test split([] {
    String s = "hello world";
    auto gen = s.split(' ');
    auto end = gen.end();
    cxx::String w;

    auto it = gen.begin();
    w = *it;
    assert(w == "hello");

    ++it;
    w = *it;
    assert(w == "world");

    ++it;
    assert(it == end);
});

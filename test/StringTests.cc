// Only include the header being tested; this single standalone include
// should work without needing other headers.
#include "cxx/String.h"

#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <iomanip>
#include <ios>
#include <iostream>

using cxx::String;
using cxx::stringdetail::Type;

namespace {

constexpr char const* kLongStringLiteral = "stringWhichHasThirtyCharacters";

struct L {
    int const n;
    friend std::ostream& operator<<(std::ostream& os, L const& x) {
        os << std::setw(x.n) << std::left;
        return os;
    }
};

struct R {
    int const n;
    friend std::ostream& operator<<(std::ostream& os, R const& x) {
        os << std::setw(x.n) << std::right;
        return os;
    }
};

void dump(char const* where, String const& s) {
    std::cerr << "@@@ " << L {40} << where << ": " << &s << ":";
    std::cerr << " type:" << int(s.data_.type());
    switch (s.data_.type()) {
    case Type::TINY:
        std::cerr
                << " (TINY)    "
                << " size_:" << R {3} << std::dec << s.data_.size_ << " ("
                << R {3} << s.size() << ")"
                << " ptr:" << R {12} << std::hex << std::intptr_t(s.data_.cstr())
                << " \"" << s.data_.cstr() << '"';
        break;

    case Type::LITERAL:
        std::cerr
                << " (LITERAL) "
                << " size_:" << R {3} << std::dec << s.data_.size_ << " ("
                << R {3} << s.size() << ")"
                << " ptr:" << R {12} << std::hex << std::intptr_t(s.data_.cstr())
                << " \"" << s.data_.cstr() << '"';
        break;

    case Type::SHARED:
        std::cerr
                << " (SHARED)  "
                << " size_:" << R {3} << std::dec << s.data_.size_ << " ("
                << R {3} << s.size() << ")"
                << " ptr:" << R {12} << std::hex << std::intptr_t(s.data_.cstr())
                << " \"" << s.data_.cstr() << '"';
        break;

    default: std::unreachable();
    }
    std::cerr << std::endl;
}

String constructAndReturnStringFromBuffer() {
    char chars[31];
    memcpy(chars, "stringWhichHasThirtyCharacters", 30);
    chars[30] = 0;
    String s = chars;
    return s;
}

}  // namespace

int main() {
    {
        constexpr String const s;
        dump("constructEmptyCStringCE", s);
        assert(0 == s.size());
        assert(s.data());
        assert(0 == strcmp("", s.data()));
    }

    {
        String const s;
        dump("constructEmptyCString", s);
        assert(0 == s.size());
        assert(s.data());
        assert(0 == strcmp("", s.data()));
    }

    {
        constexpr String s = "AAAAA";
        dump("constructTinyCStringCE", s);
        assert(5 == s.size());
        assert(0 == strcmp("AAAAA", s.data()));
    }

    {
        String s = "AAAAA";
        dump("constructTinyCString", s);
        assert(5 == s.size());
        assert(0 == strcmp("AAAAA", s.data()));
    }

    {
        constexpr String s(kLongStringLiteral);
        dump("constructCStringFromLiteralCE", s);
        assert(30 == s.size());
        assert(kLongStringLiteral == s.data());
    }

    {
        String s = kLongStringLiteral;
        dump("constructCStringFromLiteral", s);
        assert(30 == s.size());
        assert(kLongStringLiteral != s.data());
        assert(0 == strcmp(kLongStringLiteral, s.data()));
    }

    {
        char chars[31];
        memcpy(chars, "stringWhichHasThirtyCharacters", 30);
        chars[30] = 0;
        String s = chars;
        dump("constructCStringFromBuffer", s);
        assert(30 == s.size());
        assert(chars != s.data());
        assert(0 == strcmp(kLongStringLiteral, s.data()));
    }

    {
        auto const s = constructAndReturnStringFromBuffer();
        dump("constructCStringFromBufferAndReturn", s);
        assert(30 == s.size());
        assert(0 == strcmp(kLongStringLiteral, s.data()));
    }

    {
        static constexpr String s0 = kLongStringLiteral;
        dump("copyConstructCE", s0);
        constexpr String s(s0);
        dump("copyConstructCE", s);
        assert(s.size() == s0.size());
        assert(s.data() == s0.data());
    }

    {
        String s0 = kLongStringLiteral;
        dump("copyConstruct", s0);
        String s1(s0);
        dump("copyConstruct", s1);
        assert(s1.size() == s0.size());
        assert(s1.data() == s0.data());
    }

    {
        String s0;
        String s1 = kLongStringLiteral;
        dump("copyAssign (before): s0", s0);
        dump("copyAssign (before): s1", s1);
        assert(0 == s0.size());
        assert(0 == strcmp("", s0.data()));
        assert(30 == s1.size());
        assert(0 == strcmp(kLongStringLiteral, s1.data()));
        s0 = s1;
        dump("copyAssign  (after): s0", s0);
        dump("copyAssign  (after): s1", s1);
        assert(30 == s0.size());
        assert(0 == strcmp(kLongStringLiteral, s0.data()));
        assert(30 == s1.size());
        assert(0 == strcmp(kLongStringLiteral, s1.data()));
    }

    {
        constexpr String s0 = kLongStringLiteral;
        dump("moveAssign (before): s0", s0);
        assert(30 == s0.size());
        assert(0 == strcmp(kLongStringLiteral, s0.data()));
        String s1(std::move(s0));
        dump("moveAssign  (after): s0", s0);
        dump("moveAssign  (after): s1", s1);
        // NOTE: the moved-from object not dependably in any particular
        // state, for some reason, in "CE mode".
        // assert(0 == s0.size());
        // assert(0 == strcmp("", s0.data()));
        assert(30 == s1.size());
        assert(0 == strcmp(kLongStringLiteral, s1.data()));
    }

    {
        String s0 = kLongStringLiteral;
        dump("moveAssign (before): s0", s0);
        assert(30 == s0.size());
        assert(0 == strcmp(kLongStringLiteral, s0.data()));
        String s1(std::move(s0));
        dump("moveAssign  (after): s0", s0);
        dump("moveAssign  (after): s1", s1);
        assert(0 == s0.size());
        assert(0 == strcmp("", s0.data()));
        assert(30 == s1.size());
        assert(0 == strcmp(kLongStringLiteral, s1.data()));
    }

    {
        String s0 = "foo";
        String s1 = kLongStringLiteral;
        dump("moveAssign (before): s0", s0);
        dump("moveAssign (before): s1", s1);
        assert(3 == s0.size());
        assert(0 == strcmp("foo", s0.data()));
        assert(30 == s1.size());
        assert(0 == strcmp(kLongStringLiteral, s1.data()));
        s0 = std::move(s1);
        dump("moveAssign  (after): s0", s0);
        dump("moveAssign  (after): s1", s1);
        assert(30 == s0.size());
        assert(0 == strcmp(kLongStringLiteral, s0.data()));
        assert(0 == s1.size());
        assert(0 == strcmp("", s1.data()));
    }

    {
        constexpr String s1 = "foo";
        constexpr String s2 = kLongStringLiteral;
        assert(33 == (s1 + s2).size());
        assert('f' == (s1 + s2)[0]);
        assert('o' == (s1 + s2)[1]);
        assert('o' == (s1 + s2)[2]);
        assert('s' == (s1 + s2)[3]);
    }

    {
        String s1 = "foo";
        String s2 = kLongStringLiteral;
        assert(33 == (s1 + s2).size());
        assert('f' == (s1 + s2)[0]);
        assert('o' == (s1 + s2)[1]);
        assert('o' == (s1 + s2)[2]);
        assert('s' == (s1 + s2)[3]);
    }

    {
        String s = "hello world";
        auto gen = s.split(' ');
        auto end = gen.end();
        cxx::String w;

        auto it = gen.begin();
        w = *it;
        std::cerr << w << std::endl;
        assert(w == "hello");

        ++it;
        w = *it;
        std::cerr << w << std::endl;
        assert(w == "world");

        ++it;
        assert(it == end);
    }

    return 0;
}

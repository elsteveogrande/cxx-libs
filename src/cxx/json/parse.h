#pragma once
static_assert(__cplusplus >= 202300L, "cxx-libs requires C++23");
// (c) 2024 Steve O'Brien -- MIT License

#include "../Concepts.h"
#include "../algo/Expected.h"
#include "../ref/Ref.h"
#include "JSON.h"

#include <cassert>
#include <cstdint>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace cxx {

namespace detail {

template <SequenceContainerOf<char> S>
struct ParseState final {
    constexpr static char kEOF = -1;
    using I = typename S::const_iterator;

    unsigned line = 1;
    unsigned col = 1;
    I it_;
    I end_;

    explicit ParseState(S const& seq) {
        it_ = seq.begin();
        seq.end();
    }

    [[gnu::noreturn]] void error(std::string msg) { throw ParseException(line, col, ch(), msg); }

    [[gnu::noreturn]] void unexpected() { error("unexpected"); }

    char ch() { return (it_ != end_) ? *it_ : kEOF; }
    char eof() { return ch() == kEOF; }

    char read() {
        if (eof()) { error("unexpected EOF"); }
        char ret = *it_++;
        if (ret == '\n') {
            col = 1;
            ++line;
        } else {
            ++col;
        }
        return ret;
    }

    void skipSpace() {
        while (!eof()) {
            switch (ch()) {
            case ' ':
            case '\r':
            case '\n':
            case '\t': read(); break;
            default:   return;
            }
        }
    }

    void expect(char c) {
        if (ch() != c) { unexpected(); }
        read();
    }

    void expect(std::string s) {
        for (char c : s) { expect(c); }
    }

    JSON parseNull();
    JSON parseBool();
    JSON parseNumber();
    JSON parseString();
    JSON parseArray(unsigned depth);
    JSON parseObject(unsigned depth);

    void parse(std::vector<JSON>& out, unsigned depth);
};

}  // namespace detail

template <SequenceContainerOf<char> S>
Expected<JSON, ParseException> JSON::parse(S const& seq) {
    detail::ParseState ps(seq);
    try {
        std::vector<JSON> result;
        ps.parse(result, 0);
        assert(result.size() == 1);
        return result.front();
    } catch (ParseException const& exc) { return {exc}; }
}

template <SequenceContainerOf<char> S>
void detail::ParseState<S>::parse(std::vector<JSON>& out, unsigned depth) {
    skipSpace();
    auto c = ch();
    switch (c) {

    case kEOF:
        if (depth == 0 && out.size() == 1) { return; }
        error("unexpected EOF");

    case 'n':  // n(ull) token
        out.push_back(parseNull());
        break;

    case 't':  // t(rue) or
    case 'f':  // f(alse) token
        out.push_back(parseBool());
        break;

    case '"':  // open quote
        out.push_back(parseString());
        break;

    case '[':  // recurse into array
        out.push_back(parseArray(depth));
        break;

    case '{':  // recurse into object
        out.push_back(parseObject(depth));
        break;

    case ',':  // delimiter for list / object item
        if (!depth) { error("unexpected delimiter"); }
        read();  // eat the comma
        return;  // and return to array/object parsing

    default: {
        if ((c >= '0' && c <= '9') || c == '-') {
            out.push_back(parseNumber());
        } else {
            unexpected();
        }
    }
    }
}

template <SequenceContainerOf<char> S>
JSON detail::ParseState<S>::parseNull() {
    expect("null");
    return JSON {Ref<NullRepr>::make()};
}

template <SequenceContainerOf<char> S>
JSON detail::ParseState<S>::parseBool() {
    if (ch() == 't') {
        expect("true");
        return JSON {Ref<BoolRepr>::make(true)};
    }
    if (ch() == 'f') {
        expect("false");
        return JSON {Ref<BoolRepr>::make(false)};
    }
    std::unreachable();
}

template <SequenceContainerOf<char> S>
JSON detail::ParseState<S>::parseNumber() {
    auto digit = [&] { return ch() >= '0' && ch() <= '9'; };

    int64_t sign = 1;
    int64_t intPart = 0;
    double fracTop = 0.0;
    double fracBot = 1.0;

    if (ch() == '-') {
        sign = -1;
        read();
    }
    while (digit()) {
        intPart *= 10;
        intPart += (ch() - '0');
        read();
    }
    if (ch() == '.') {
        read();
        while (digit()) {
            fracTop *= 10.0;
            fracBot *= 10.0;
            fracTop += ch() - '0';
            read();
        }
    }

    // TODO exponents

    return sign * (intPart + (fracTop / fracBot));
}

template <SequenceContainerOf<char> S>
JSON detail::ParseState<S>::parseString() {
    // TODO escaped sequences!
    std::stringstream ss;
    expect('"');           // consume open quote
    while (ch() != '"') {  // until we reach the close quote
        ss << read();      // consume char
    }
    expect('"');  // consume closing quote
    return ss.str();
}

template <SequenceContainerOf<char> S>
JSON detail::ParseState<S>::parseArray(unsigned depth) {
    expect('[');              // consume open bracket
    std::vector<JSON> items;  // build up a list of JSONs
    goto readItems;

done:
    read();                          // consume closing bracket
    return JSON {items};             // return array
readItems:                           // loop to get zero or more items
    skipSpace();                     //
    if (ch() == ']') { goto done; }  // closing bracket: done
    parse(items, depth + 1);         // get a value (of any JSON type)
    if (ch() == ']') { goto done; }  // closing bracket: done
    skipSpace();                     //
    if (ch() == ',') {               // comma: loop to get more
        read();                      // consume
        goto readItems;              // loop
    }
    unexpected();
}

template <SequenceContainerOf<char> S>
JSON detail::ParseState<S>::parseObject(unsigned depth) {
    expect('{');                         // consume open bracket
    auto ret = Ref<ObjectRepr>::make();  // build up a list of properties
    goto readProps;

done:
    read();                                                 // consume closing brace
    return JSON {ret};                                      // done with object
readProps:                                                  // loop to get zero or more props
    skipSpace();                                            //
    if (ch() == '}') { goto done; }                         // closing brace: done
    auto key = parseString();                               // get the key;
    auto& keyRepr = dynamic_cast<StringRepr&>(*key.repr_);  // we're expecting a string
    auto keyStr = keyRepr.val_;                             // actual string
    skipSpace();                                            //
    expect(':');                                            // consume colon
    std::vector<JSON> vals;                                 // "list" of values, expect only 1
    parse(vals, depth + 1);                                 // get the value (of any JSON type)
    if (vals.size() != 1) { error("expected key:val"); }    // should have exactly 1 val
    auto val = std::move(vals.front());                     // get that 1 value (JSON)
    auto prop = Ref<ObjectProp>::make(keyStr, val);         // make a new prop (ref to ObjectProp)
    ret->vec_.pushBack(prop);                               // push to this object's prop vector
    skipSpace();                                            //
    if (ch() == '}') { goto done; }                         // closing brace: done
    if (ch() == ',') {                                      // comma: loop to get more props
        read();                                             // consume
        goto readProps;                                     // loop
    }
    unexpected();
}

}  // namespace cxx

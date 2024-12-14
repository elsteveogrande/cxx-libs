#pragma once
static_assert(__cplusplus >= 202300L, "cxx-libs requires C++23");
// (c) 2024 Steve O'Brien -- MIT License

#include "cxx/Generator.h"
#include "cxx/Ref.h"
#include "cxx/String.h"

#include <cassert>
#include <concepts>
#include <cstddef>
#include <iostream>
#include <ranges>
#include <string>
#include <type_traits>
#include <utility>

namespace cxx {

/** For `JSONString`: anything you can pass to a cxx::String constructor */
template <typename T>
concept Stringable = requires(T&& val) { cxx::String(val); };

/**
 * For `JSONObject`: some kind of mapping, resembling an AssociativeContainer
 * (however this concept is not thorough, only checking for `mapped_type`).
 * https://en.cppreference.com/w/cpp/named_req/AssociativeContainer
 */
template <typename A>
concept AssocAny = requires(A&& assoc) { typename A::mapped_type; };

/**
 * For `JSONObject`: like `AssocAny` but has `Stringable` keys.
 */
template <typename A>
concept AssocStringToAny = requires(A&& assoc) {
    typename A::mapped_type;
    Stringable<typename A::key_type>;
};

/** For `JSONObject`: something that can generate key-value pairs representing itself. */
template <typename T>
concept CanGenJSONProps = requires(T&& obj) { obj.genJSONProps(); };

/**
 * For `JSONArray`: something that looks like a `ranges::view` and / or has `value_type`,
 * which includes `Generator`s; with several exceptions: exclude things that can be
 * iterated over but are map-like (they should be JSONObjects instead), or are
 * string-like (they should be JSONStrings).
 */
template <typename S>
concept SequenceAny =
        (!AssocAny<S>) && (!CanGenJSONProps<S>) && (!Stringable<S>) &&
        (std::ranges::view<S> || requires(S& seq) { typename S::value_type; });

class JSON;
struct JSONProp;

class JSONBase : public RefCounted<JSONBase> {
public:
    virtual ~JSONBase() = default;
    virtual void write(std::ostream& os) const = 0;
};

class JSONNull final : public JSONBase {
public:
    void write(std::ostream& os) const override { os << "null"; }
};

class JSONBool final : public JSONBase {
    bool val_;

public:
    void write(std::ostream& os) const override { os << (val_ ? "true" : "false"); }
    JSONBool(bool val) : val_(val) {}
};

class JSONNumber final : public JSONBase {
    double val_;

public:
    void write(std::ostream& os) const override { os << val_; }
    JSONNumber(double val) : val_(val) {}
};

class JSONString final : public JSONBase {
    cxx::String val_ = "";  // to make this default-constructible

public:
    JSONString(Stringable auto const& val) : val_ {val} {}

    operator cxx::String() const { return val_; }

    void write(std::ostream& os) const override { os << '"' << val_ /*TODO*/ << '"'; }
};

class JSON final {
    Ref<JSONBase> json_;

public:
    void write(std::ostream& os) const { json_->write(os); }

    JSON(JSON const& rhs) = default;
    JSON& operator=(JSON const& rhs) = default;

    // JSONNull (produced by default-construction)
    JSON() : JSON(nullptr) {}
    JSON(std::nullptr_t) : json_(make<JSONNull>()) {}

    // JSONBool (has to be literally a `bool`, not something `operator bool`-able)
    template <typename T>
        requires std::same_as<T, bool> JSON(T val) : json_(make<JSONBool>(val)) {}

    // JSONNumber: anything convertible to a number, except `bool`s
    template <typename T>
        requires(!std::same_as<T, bool>) && (std::is_integral_v<T> || std::is_floating_point_v<T>)
    JSON(T val) : json_(make<JSONNumber>(val)) {}

    // JSONString: anything that cxx::String ctors accept
    JSON(Stringable auto const& val) : json_(make<JSONString>(val)) {}

    // JSONArray: any sequence (anything with [c]begin, [c]end), range, or view (including
    // `Generator`s), with several exclusions; see `SequenceAny`.
    JSON(SequenceAny auto const& seq);

    // JSONObject: accepts something that can generate properties (key-value pairs)
    JSON(CanGenJSONProps auto const& obj);

    // JSONObject: accept something resembling a `map` or similar container, with `Stringable`
    // keys. See: https://en.cppreference.com/w/cpp/named_req/AssociativeContainer
    JSON(AssocStringToAny auto const& map);

    // Can be applied to views to automagically convert JSON-able things to JSON
    constexpr static auto transform = std::views::transform([](auto&& val) { return JSON(val); });
};
static_assert(std::semiregular<JSON>);

template <typename T>
concept JSONable = std::semiregular<T> && requires(T val) { JSON(val); };

class JSONArray final : public JSONBase {
    Generator<JSON> genItems;

public:
    void write(std::ostream& os) const override;

    static Generator<JSON> asGenerator(auto it, auto end) {
        for (; it != end; ++it) { co_yield *it; }
    }

    template <typename G>
        requires Generates<G, JSON> JSONArray(G gen) : genItems(gen) {}

    template <typename G>
        requires GeneratesAny<G> && (!Generates<G, JSON>)
    JSONArray(G gen) : JSONArray(gen | JSON::transform) {}

    template <typename S>
        requires SequenceAny<S> && (!GeneratesAny<S>)
    JSONArray(S const& seq) : JSONArray(asGenerator(seq.begin(), seq.end())) {}
};

struct JSONProp final {
    JSONString name_;
    JSON val_;

    // Ensure this class is default-conclassible
    JSONProp() : name_(""), val_(JSONString("")) {}

    JSONProp(JSONString name, JSON val) : name_(std::move(name)), val_(std::move(val)) {}
    JSONProp(cxx::String name, JSON val) : name_(JSONString(name)), val_(std::move(val)) {}
    JSONProp(std::string name, JSON val) : name_(cxx::String(name)), val_(std::move(val)) {}
    JSONProp(char const* name, JSON val) : name_(cxx::String(name)), val_(std::move(val)) {}
};
static_assert(std::semiregular<JSONProp>);

class JSONObject final : public JSONBase {
    cxx::Generator<JSONProp> genJSONProps;

    static cxx::Generator<JSONProp> genJSONPropsFor(AssocStringToAny auto const& map) {
        for (auto const& entry : map) { co_yield {{entry.first}, JSON(entry.second)}; }
    }

public:
    void write(std::ostream& os) const override;

    JSONObject(CanGenJSONProps auto const& obj) : genJSONProps(obj.genJSONProps()) {};

    JSONObject(AssocStringToAny auto const& map) : genJSONProps(genJSONPropsFor(map)) {};
};

JSON::JSON(SequenceAny auto const& seq) : json_(make<JSONArray>(seq)) {}

JSON::JSON(CanGenJSONProps auto const& obj) : json_(make<JSONObject>(obj)) {}

JSON::JSON(AssocStringToAny auto const& obj) : json_(make<JSONObject>(obj)) {}

inline void JSONArray::write(std::ostream& os) const {
    os << '[';
    std::string sep = "";
    for (JSON item : genItems) {
        os << sep;
        sep = ",";
        item.write(os);
    }
    os << ']';
}

inline void JSONObject::write(std::ostream& os) const {
    cxx::String sep = "{";
    for (auto prop : genJSONProps) {
        os << sep;
        sep = ",";
        prop.name_.write(os);
        os << ':';
        prop.val_.write(os);
    }
    os << '}';
}

}  // namespace cxx

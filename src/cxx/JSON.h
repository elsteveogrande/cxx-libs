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
#include <string>
#include <type_traits>
#include <utility>

namespace cxx {

template <typename C, typename I>
concept Container = requires(C const& c) {
    c.begin();
    c.end();
};

template <typename A, typename V>
concept Assoc = Container<A, V> && requires(A const& a) {
    std::is_same_v<typename A::mapped_type, V>;
    a.cbegin()->first;
    a.cbegin()->second;
};

template <typename T>
concept HasGenJSONProps = requires(T const& obj) { obj.genJSONProps(); };

class JSON;

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
    void write(std::ostream& os) const override {
        os << (val_ ? "true" : "false");
    }
    JSONBool(bool val)
            : val_(val) {}
};

class JSONNumber final : public JSONBase {
    double val_;

public:
    void write(std::ostream& os) const override { os << val_; }
    JSONNumber(double val)
            : val_(val) {}
};

class JSONString final : public JSONBase {
    cxx::String val_ = "";  // to make this default-constructible

public:
    JSONString(cxx::String val)
            : val_(val) {}
    operator cxx::String() const { return val_; }

    void write(std::ostream& os) const override {
        os << '"' << val_ /*TODO*/ << '"';
    }
};

class JSONArray final : public JSONBase {
    cxx::Generator<JSON> genItems;

public:
    void write(std::ostream& os) const override;

    template <typename X>
        requires requires(X x) { JSON(x); } static JSON toJSON(X val);

    template <typename I, typename J, typename T = typename I::value_type>
    JSONArray(I it, J end);
};

struct JSONProp;

class JSONObject final : JSONBase {
    cxx::Generator<JSONProp> genJSONProps;

public:
    void write(std::ostream& os) const override;

    template <HasGenJSONProps T>
    JSONObject(T const& obj)
            : genJSONProps(obj.genJSONProps()) {};
};

class JSON final {
    Ref<JSONBase> json_;

public:
    void write(std::ostream& os) const { json_->write(os); }

    // JSON() : json_(nullptr) {}

    // template <typename J> JSON(Ref<J> json) : json_(json) {}

    JSON()
            : json_(make<JSONNull>()) {}

    JSON(std::nullptr_t)
            : json_(make<JSONNull>()) {}

    template <typename T>
        requires std::same_as<T, bool> JSON(T val)
            : json_(make<JSONBool>(val)) {}

    template <typename T>
        requires(!std::same_as<T, bool>) &&
                (std::is_integral_v<T> || std::is_floating_point_v<T>)
    JSON(T val)
            : json_(make<JSONNumber>(val)) {}

    JSON(cxx::String const& val)
            : json_(make<JSONString>(val)) {}

    template <typename T>
        requires requires(T const& val) { val.toString(); } JSON(T const& val)
            : json_(make<JSONString>(val.toString())) {}

    // template <typename T> requires requires(T const& val) {
    //     val.genJSONProps();
    // } JSON(T const& val) : json_(make<JSONObject>(val)) {}

    template <typename T>
        requires requires(T const& t) {
            t.begin();
            t.end();
        } JSON(T val)
            : json_(make<JSONArray>(val.begin(), val.end())) {}
};

struct JSONProp final {
    JSONString name_;
    JSON val_;

    // Ensure this class is default-conclassible
    JSONProp()
            : name_(""), val_(JSONString("")) {}

    JSONProp(JSONString name, JSON val)
            : name_(std::move(name)), val_(std::move(val)) {}
    JSONProp(cxx::String name, JSON val)
            : name_(JSONString(name)), val_(std::move(val)) {}
    JSONProp(std::string name, JSON val)
            : name_(cxx::String(name)), val_(std::move(val)) {}
    JSONProp(char const* name, JSON val)
            : name_(cxx::String(name)), val_(std::move(val)) {}
};

template <typename X>
    requires requires(X x) { JSON(x); } JSON JSONArray::toJSON(X x) {
    return {x};
}

template <typename I, typename J, typename T>
JSONArray::JSONArray(I it, J end)
        : genItems(Generator<T>::of(it, end).map(toJSON<T>)) {}

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

#pragma once
static_assert(__cplusplus >= 202300L, "cxx-libs requires C++23");
// (c) 2024 Steve O'Brien -- MIT License

#include "cxx/Generator.h"
#include "cxx/Ref.h"
#include "cxx/String.h"

#include <cassert>
#include <cstddef>
#include <iostream>
#include <memory>
#include <optional>
#include <type_traits>
#include <vector>

namespace cxx {

/** Something that's literally a `bool` and not just convertible to a `bool` */
template <typename B>
concept Bool = std::is_same_v<std::remove_cv_t<B>, bool>;

/** Something accepted by a `String` constructor */
template <typename S>
concept Stringable = requires { String(S()); };

/** A map with string keys */
template <typename M>
concept ObjectConvertible = requires {
    Stringable<typename M::key_type>;
    typename M::mapped_type;
};

/** Something iterable, but neither a map nor a string */
template <typename S>
concept JSONArrayConvertible = (!ObjectConvertible<S>) && (!std::is_same_v<typename S::value_type, char>) && requires {
    typename S::value_type;
} && !requires { S().has_value(); };

struct JSON;
struct ObjectProp;
struct Repr;
struct NullRepr;
struct BoolRepr;
struct NumRepr;
struct StringRepr;
struct ArrayRepr;
struct ObjectRepr;

struct JSON final {
    std::shared_ptr<Repr> val_;

    ~JSON() = default;
    JSON() : JSON(nullptr) {}
    JSON(nullptr_t);
    JSON(Bool auto val);
    JSON(double val);
    JSON(Stringable auto val);
    JSON(JSONArrayConvertible auto const& val);
    JSON(Generator<ObjectProp> gen);
    JSON(ObjectConvertible auto map);

    template <typename T>
    JSON(std::optional<T> val);

    bool operator==(JSON const& rhs) const;

    std::ostream& write(std::ostream& os) const;

    template <typename T>
    static std::shared_ptr<Repr> valueOrNull(std::optional<T> maybe);
};

struct Repr {
    virtual ~Repr() = default;
    virtual bool operator==(Repr const&) const { return false; }
    virtual void write(std::ostream& os) const = 0;
};

struct NullRepr : Repr {
    bool operator==(NullRepr const&) const { return true; }
    void write(std::ostream& os) const override { os << "null"; }
};

struct BoolRepr : Repr {
    bool val_;
    BoolRepr(Bool auto val) : val_(val) {}
    bool operator==(BoolRepr const& rhs) const { return val_ == rhs.val_; }
    void write(std::ostream& os) const override { os << (val_ ? "true" : "false"); }
};

struct NumRepr : Repr {
    double val_;
    NumRepr(double val) : val_(val) {}
    bool operator==(NumRepr const& rhs) const { return val_ == rhs.val_; }
    void write(std::ostream& os) const override { os << val_; }
};

struct StringRepr : Repr {
    String val_;
    StringRepr(Stringable auto val) : val_(String(val)) {}
    bool operator==(StringRepr const& rhs) const { return val_ == rhs.val_; }
    void write(std::ostream& os) const override { os << '"' << val_ << '"'; }
};

struct ArrayRepr : Repr {
    std::shared_ptr<std::vector<JSON>> val_;
    ArrayRepr();
    static bool arraysEqual(auto& a1, auto& a2);
    bool operator==(ArrayRepr const& rhs) const { return arraysEqual(*val_, *rhs.val_); }

    void write(std::ostream& os) const override;
};

struct ObjectProp final {
    String key;
    JSON val;
    bool operator==(ObjectProp const& rhs) const;
};

struct ObjectRepr : Repr {
    std::shared_ptr<std::vector<ObjectProp>> val_;
    ObjectRepr();
    static bool objectsEqual(auto& a1, auto& a2);
    bool operator==(ObjectRepr const& rhs) const { return objectsEqual(*val_, *rhs.val_); }

    void write(std::ostream& os) const override;
};

ArrayRepr::ArrayRepr() : val_(std::make_shared<std::vector<JSON>>()) {}

void ArrayRepr::write(std::ostream& os) const {
    char sep = '[';
    for (auto item : *val_) {
        os << sep;
        item.write(os);
        sep = ',';
    }
    os << ']';
}

bool ArrayRepr::arraysEqual(auto& a1, auto& a2) {
    if (a1.size() != a2.size()) { return false; }
    auto end = a1.end();
    auto it1 = a1.begin();
    auto it2 = a2.begin();
    while (it1 != end) {
        if (*it1++ != *it2++) { return false; }
    }
    return true;
}

ObjectRepr::ObjectRepr() : val_(std::make_shared<std::vector<ObjectProp>>()) {}

bool ObjectRepr::objectsEqual(auto& a1, auto& a2) {
    if (a1.size() != a2.size()) { return false; }
    auto end = a1.end();
    auto it1 = a1.begin();
    auto it2 = a2.begin();
    while (it1 != end) {
        if (*it1++ != *it2++) { return false; }
    }
    return true;
}

void ObjectRepr::write(std::ostream& os) const {
    char sep = '{';
    for (auto item : *val_) {
        os << sep;
        auto key = JSON(item.key);  // convert to a JSON string, print it
        key.write(os);
        os << ':';
        auto v = item.val;  // already a Ref<JSON>
        v.write(os);
        sep = ',';
    }
    os << '}';
}

JSON::JSON(nullptr_t) : val_(std::make_shared<NullRepr>()) {}
JSON::JSON(Bool auto val) : val_(std::make_shared<BoolRepr>(val)) {}
JSON::JSON(double val) : val_(std::make_shared<NumRepr>(val)) {}
JSON::JSON(Stringable auto val) : val_(std::make_shared<StringRepr>(val)) {}

template <typename T>
std::shared_ptr<Repr> JSON::valueOrNull(std::optional<T> maybe) {
    if (!maybe.has_value()) { return std::make_shared<NullRepr>(); }
    return (JSON(maybe.value()).val_);
}

template <typename T>
JSON::JSON(std::optional<T> val) : val_(std::move(valueOrNull(val))) {}

JSON::JSON(JSONArrayConvertible auto const& val) : val_(std::make_shared<ArrayRepr>()) {
    auto arr = dynamic_cast<ArrayRepr&>(*val_);
    for (auto item : val) { arr.val_->push_back(JSON(item)); }
}

std::ostream& JSON::write(std::ostream& os) const {
    val_->write(os);
    return os;
}

bool JSON::operator==(JSON const& rhs) const { return *val_ == *rhs.val_; }

JSON::JSON(Generator<ObjectProp> gen) : val_(std::make_shared<ObjectRepr>()) {
    auto& obj = dynamic_cast<ObjectRepr&>(*val_);
    for (auto const& prop : gen) { obj.val_->push_back(prop); }
}

JSON::JSON(ObjectConvertible auto map) : val_(std::make_shared<ObjectRepr>()) {
    auto& obj = dynamic_cast<ObjectRepr&>(*val_);
    for (auto [key, val] : map) { obj.val_->push_back({key, JSON(val)}); }
}

bool ObjectProp::operator==(ObjectProp const& rhs) const {
    return (key == rhs.key) && (val == rhs.val);
}

}  // namespace cxx

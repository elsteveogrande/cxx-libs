#pragma once
static_assert(__cplusplus >= 202300L, "cxx-libs requires C++23");
// (c) 2024 Steve O'Brien -- MIT License

#include "decl/_JSONBase.h"
#include "decl/_LinkedList.h"

#include <cassert>
#include <cstddef>
#include <cxx/Generator.h>
#include <cxx/String.h>
#include <memory>
#include <optional>

namespace cxx {

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

JSON::JSON(nullptr_t) : JSON(std::make_shared<NullRepr>()) {}
JSON::JSON(Bool auto val) : JSON(std::make_shared<BoolRepr>(val)) {}
JSON::JSON(Numberish auto val) : JSON(std::make_shared<NumRepr>(val)) {}
JSON::JSON(Stringable auto val) : JSON(std::make_shared<StringRepr>(val)) {}

template <typename T>
JSON JSON::valueOrNull(std::optional<T> const& maybe) {
    if (!maybe.has_value()) { return {}; }
    return {maybe.value()};
}

template <typename T>
JSON::JSON(std::optional<T> val) : JSON(valueOrNull(val)) {}

JSON::JSON(JSONArrayConvertible auto const& val) : JSON(std::make_shared<ArrayRepr>()) {
    auto& arr = dynamic_cast<ArrayRepr&>(*repr_);
    for (auto const& item : val) { arr.vec_.pushBack(std::make_shared<JSON>(item)); }
}

bool JSON::operator==(JSON const& rhs) const { return *repr_ == *rhs.repr_; }

JSON::JSON(Generator<ObjectProp> gen) : JSON(std::make_shared<ObjectRepr>()) {
    auto& obj = dynamic_cast<ObjectRepr&>(*repr_);
    for (auto const& prop : gen) { obj.vec_.pushBack(std::make_shared<ObjectProp>(prop)); }
}

JSON::JSON(ObjectConvertible auto const& map) : JSON(std::make_shared<ObjectRepr>()) {
    auto& obj = dynamic_cast<ObjectRepr&>(*repr_);
    for (auto [key, val] : map) { obj.vec_.pushBack(std::make_shared<ObjectProp>(key, JSON(val))); }
}

bool ObjectProp::operator==(ObjectProp const& rhs) const {
    return (key_ == rhs.key_) && (val_ == rhs.val_);
}

}  // namespace cxx

#include "decl/_JSONRead.h"
#include "decl/_JSONWrite.h"

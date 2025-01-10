#pragma once
static_assert(__cplusplus >= 202300L, "cxx-libs requires C++23");
// (c) 2024 Steve O'Brien -- MIT License

#include "decl/_JSONBase.h"
#include "decl/_LinkedList.h"
#include "decl/ref/base.h"

#include <cassert>
#include <cstddef>
#include <cxx/Generator.h>
#include <cxx/String.h>
#include <optional>

namespace cxx {

struct JSON;

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

JSON::JSON(nullptr_t) : JSON(Ref<NullRepr>::make()) {}
JSON::JSON(Bool auto val) : JSON(Ref<BoolRepr>::make(val)) {}
JSON::JSON(Numberish auto val) : JSON(Ref<NumRepr>::make(val)) {}
JSON::JSON(Stringable auto val) : JSON(Ref<StringRepr>::make(val)) {}

template <typename T>
JSON JSON::valueOrNull(std::optional<T> const& maybe) {
    if (!maybe.has_value()) { return {}; }
    return {maybe.value()};
}

template <typename T>
JSON::JSON(std::optional<T> val) : JSON(valueOrNull(val)) {}

JSON::JSON(JSONArrayConvertible auto const& val) : JSON(Ref<ArrayRepr>::make()) {
    auto& arr = dynamic_cast<ArrayRepr&>(*repr_);
    for (auto const& item : val) { arr.vec_.pushBack(Ref<JSON>::make(item)); }
}

bool JSON::operator==(JSON const& rhs) const { return *repr_ == *rhs.repr_; }

JSON::JSON(Generator<ObjectProp> gen) : JSON(Ref<ObjectRepr>::make()) {
    auto& obj = dynamic_cast<ObjectRepr&>(*repr_);
    for (auto const& prop : gen) { obj.vec_.pushBack(Ref<ObjectProp>::make(prop)); }
}

JSON::JSON(ObjectConvertible auto const& map) : JSON(Ref<ObjectRepr>::make()) {
    auto& obj = dynamic_cast<ObjectRepr&>(*repr_);
    for (auto [key, val] : map) { obj.vec_.pushBack(Ref<ObjectProp>::make(key, JSON(val))); }
}

bool ObjectProp::operator==(ObjectProp const& rhs) const {
    return (key_ == rhs.key_) && (val_ == rhs.val_);
}

}  // namespace cxx

#include "decl/_JSONRead.h"
#include "decl/_JSONWrite.h"

#include <cxx/Ref.h>

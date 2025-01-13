#pragma once
static_assert(__cplusplus >= 202300L, "cxx-libs requires C++23");
// (c) 2024 Steve O'Brien -- MIT License

#include "Concepts.h"
#include "algo/LinkedList.h"
#include "gen/Generator.h"
#include "ref/Ref.h"
#include "string/String.h"

#include "json/JSON.h"
#include <cassert>
#include <cstddef>
#include <optional>

namespace cxx {

// Declare types here so IDE considers this file (not a decl/ file) "authoritative"
struct JSON;
struct ObjectProp;

JSON::JSON(nullptr_t) : JSON(Ref<NullRepr>::make()) {}
JSON::JSON(Bool auto val) : JSON(Ref<BoolRepr>::make(val)) {}
JSON::JSON(NumericNotBool auto val) : JSON(Ref<NumRepr>::make(val)) {}
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

#include "json/parse.h"
#include "json/write.h"
#include <cxx/Ref.h>

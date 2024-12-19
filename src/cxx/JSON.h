#pragma once
static_assert(__cplusplus >= 202300L, "cxx-libs requires C++23");
// (c) 2024 Steve O'Brien -- MIT License

#include "cxx/Generator.h"
#include "cxx/Ref.h"
#include "cxx/String.h"

#include <cassert>
#include <cstddef>
#include <iostream>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>

namespace cxx {

/** Something that's literally a `bool` and not just convertible to a `bool` */
template <typename B>
concept Bool = std::is_same_v<std::remove_cv_t<B>, bool>;

/** Something accepted by a `cxx::String` constructor */
template <typename S>
concept Stringable = requires { cxx::String(S()); };

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
};

//     // Can be applied to views to automagically convert JSON-able things to JSON
//     constexpr static auto transform = std::views::transform([](auto&& val) { return JSON(val);
//     });

struct JSON final : RefCounted<JSON> {
    using Array = std::vector<Ref<JSON>>;

    struct ObjectProp final {
        cxx::String key;
        Ref<JSON> val;
    };

    using Object = std::vector<ObjectProp>;

    std::variant<nullptr_t, bool, double, cxx::String, Array, Object> val_;

    ~JSON() = default;
    JSON() = default;

    JSON(nullptr_t) : val_(nullptr) {}
    JSON(Bool auto val) : val_(val) {}
    JSON(double val) : val_(val) {}
    JSON(Stringable auto val) : val_(val) {}

    JSON(JSONArrayConvertible auto val) : val_(Array()) {
        auto arr = Array();
        for (auto item : val) { arr.push_back(cxx::make<JSON>(item)); }
        val_ = std::move(arr);
    }

    JSON(cxx::Generator<ObjectProp> gen) : val_(Object()) {
        auto obj = Object();
        for (auto prop : gen) { obj.push_back(prop); }
        val_ = std::move(obj);
    }

    JSON(ObjectConvertible auto map) : val_(Object()) {
        auto obj = Object();
        for (auto [key, val] : map) { obj.push_back({key, cxx::make<JSON>(val)}); }
        val_ = std::move(obj);
    }

    std::ostream& write(std::ostream& os) const;
};

struct JSONWriter {
    std::ostream& os;

    void operator()(nullptr_t) { os << "null"; }
    void operator()(bool val) { os << (val ? "true" : "false"); }
    void operator()(double val) { os << val; }
    void operator()(cxx::String val) { os << '"' << val << '"'; }

    void operator()(JSON::Array const& val) {
        char sep = '[';
        for (auto item : val) {
            os << sep;
            item->write(os);
            sep = ',';
        }
        os << ']';
    }

    void operator()(JSON::Object const& val) {
        char sep = '{';
        for (auto item : val) {
            os << sep;
            auto key = JSON(item.key);  // convert to a JSON string, print it
            key.write(os);
            os << ':';
            auto val = item.val;  // already a Ref<JSON>
            val->write(os);
            sep = ',';
        }
        os << '}';
    }
};

std::ostream& JSON::write(std::ostream& os) const {
    std::visit(JSONWriter {os}, val_);
    return os;
}

}  // namespace cxx

#pragma once
static_assert(__cplusplus >= 202300L, "cxx-libs requires C++23");
// (c) 2024 Steve O'Brien -- MIT License

#include "_LinkedList.h"

#include <cassert>
#include <cstddef>
#include <cxx/Generator.h>
#include <cxx/String.h>
#include <iostream>
#include <memory>
#include <optional>
#include <type_traits>
#include <utility>

namespace cxx {

/** Something that's literally a `bool` and not just convertible to a `bool` */
template <typename B>
concept Bool = std::is_same_v<std::remove_cv_t<B>, bool>;

/** Something accepted by a `String` constructor */
template <typename S>
concept Stringable = requires { String(S()); };

template <typename N>
concept Numberish = (!Bool<N>) && (std::is_integral_v<N> || std::is_floating_point_v<N>);

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

struct Repr {
    virtual ~Repr() noexcept = default;
    virtual bool operator==(Repr const&) const { return false; }
    virtual void write(std::ostream& os) const = 0;
};

struct JSON final {
    std::shared_ptr<Repr> repr_;

    template <typename R>
    explicit JSON(std::shared_ptr<R> repr) : repr_(std::move(repr)) {}

    JSON() : JSON(nullptr) {}
    ~JSON() noexcept = default;
    JSON(JSON const&) = default;
    JSON& operator=(JSON const&) = default;

    JSON(nullptr_t);
    JSON(Bool auto val);
    JSON(Numberish auto val);
    JSON(Stringable auto val);
    JSON(JSONArrayConvertible auto const& val);
    JSON(Generator<ObjectProp> gen);
    JSON(ObjectConvertible auto const& map);

    template <typename T>
    JSON(std::optional<T> val);

    Repr const& val() const noexcept;

    bool operator==(JSON const& rhs) const;

    std::ostream& write(std::ostream& os) const;

    template <typename T>
    static JSON valueOrNull(std::optional<T> const& maybe);
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
    void write(std::ostream& os) const override { os << '"' << val_ << '"'; }  // TODO FIX!
};

struct ArrayRepr : Repr {
    LinkedList<JSON> vec_;

    ~ArrayRepr() noexcept = default;
    ArrayRepr() noexcept = default;

    static bool arraysEqual(auto& a1, auto& a2);
    bool operator==(ArrayRepr const& rhs) const { return arraysEqual(vec_, rhs.vec_); }

    void write(std::ostream& os) const override;
};

struct ObjectProp final {
    String key_;
    JSON val_;

    ~ObjectProp() noexcept = default;
    ObjectProp() noexcept = default;
    ObjectProp(String key, JSON const& val) noexcept : key_(std::move(key)), val_(val) {}
    ObjectProp(String key, JSON&& val) noexcept : key_(std::move(key)), val_(std::move(val)) {}
    bool operator==(ObjectProp const& rhs) const;
};

struct ObjectRepr : Repr {
    LinkedList<ObjectProp> vec_;

    ~ObjectRepr() noexcept = default;
    ObjectRepr() noexcept = default;

    void write(std::ostream& os) const override;

    static bool objectsEqual(auto& a1, auto& a2);
    bool operator==(ObjectRepr const& rhs) const noexcept { return objectsEqual(vec_, rhs.vec_); }
};

}  // namespace cxx

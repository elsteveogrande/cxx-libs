#pragma once
static_assert(__cplusplus >= 202300L, "cxx-libs requires C++23");
// (c) 2024 Steve O'Brien -- MIT License

#include "../Concepts.h"
#include "../algo/Expected.h"
#include "../algo/LinkedList.h"
#include "../exc/Exception.h"
#include "../gen/Generator.h"
#include "../ref/Ref.h"
#include "../string/String.h"
#include "../string/compare.h"

#include <cassert>
#include <cstddef>
#include <iostream>
#include <optional>
#include <string>
#include <type_traits>
#include <utility>

namespace cxx {

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

struct ParseException final : Exception<ParseException> {
    ~ParseException() = default;
    ParseException(int line, int col, char ch, std::string desc) : Exception() {
        *this << "JSON parse error at line " << line << ", column " << col << ", at character '"
              << ch << "': " << desc;
    }
};

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

    virtual bool operator==(Repr const& rhs) const final {
        if (typeid(rhs) == typeid(*this)) { return equals(rhs); }
        return false;
    }

    virtual bool equals(Repr const& rhs) const = 0;

    virtual void write(std::ostream& os) const = 0;
};

struct JSON final {
    Ref<Repr> repr_;

    template <typename R>
    explicit JSON(Ref<R> repr) : repr_(std::move(repr)) {}

    JSON() : JSON(nullptr) {}
    ~JSON() noexcept = default;
    JSON(JSON const&) = default;
    JSON& operator=(JSON const&) = default;

    JSON(nullptr_t);
    JSON(Bool auto val);
    JSON(NumericNotBool auto val);
    JSON(Stringable auto val);
    JSON(JSONArrayConvertible auto const& val);
    JSON(Generator<ObjectProp> gen);
    JSON(ObjectConvertible auto const& map);

    template <typename T>
    static JSON valueOrNull(std::optional<T> const& maybe);

    template <typename T>
    JSON(std::optional<T> val);

    Repr const& val() const noexcept;

    bool operator==(JSON const& rhs) const;

    // write.h
    std::ostream& write(std::ostream& os) const;
    cxx::String str() const;

    // parse.h
    template <SequenceContainerOf<char> S>
    static Expected<JSON, ParseException> parse(S const& seq);
};

struct NullRepr : Repr {
    auto const* cast(Repr const& repr) const { return dynamic_cast<NullRepr const*>(&repr); }
    bool equals(Repr const& rhs) const override { return dynamic_cast<NullRepr const*>(&rhs); }
    void write(std::ostream& os) const override { os << "null"; }
};

struct BoolRepr : Repr {
    bool val_;
    BoolRepr(Bool auto val) : val_(val) {}
    auto const* cast(Repr const& repr) const { return dynamic_cast<BoolRepr const*>(&repr); }
    bool equals(Repr const& rhs) const override { return val_ == cast(rhs)->val_; }
    void write(std::ostream& os) const override { os << (val_ ? "true" : "false"); }
};

struct NumRepr : Repr {
    double val_;
    NumRepr(double val) : val_(val) {}
    auto const* cast(Repr const& repr) const { return dynamic_cast<NumRepr const*>(&repr); }
    bool equals(Repr const& rhs) const override { return val_ == cast(rhs)->val_; }
    void write(std::ostream& os) const override { os << val_; }
};

struct StringRepr : Repr {
    String val_;
    StringRepr(Stringable auto val) : val_(String(val)) {}
    auto const* cast(Repr const& repr) const { return dynamic_cast<StringRepr const*>(&repr); }
    bool equals(Repr const& rhs) const override { return cast(rhs) && val_ == cast(rhs)->val_; }
    void write(std::ostream& os) const override { os << '"' << val_ << '"'; }  // TODO FIX!
};

struct ArrayRepr : Repr {
    LinkedList<JSON> vec_;

    ~ArrayRepr() noexcept = default;
    ArrayRepr() noexcept = default;

    void write(std::ostream& os) const override;
    auto const* cast(Repr const& repr) const { return dynamic_cast<ArrayRepr const*>(&repr); }
    bool equals(Repr const& rhs) const override { return cast(rhs) && (vec_ == cast(rhs)->vec_); }
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
    auto const* cast(Repr const& repr) const { return dynamic_cast<ObjectRepr const*>(&repr); }
    bool equals(Repr const& rhs) const override { return cast(rhs) && (vec_ == cast(rhs)->vec_); }
};

}  // namespace cxx

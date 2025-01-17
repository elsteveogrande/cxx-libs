#pragma once
static_assert(__cplusplus >= 202300L, "cxx-libs requires C++23");
// (c) 2024 Steve O'Brien -- MIT License

#include "../algo/LinkedList.h"
#include "../string/String.h"
#include "JSON.h"

#include <cassert>
#include <cstddef>
#include <iostream>
#include <sstream>

namespace cxx {

std::ostream& JSON::write(std::ostream& os) const {
    repr_->write(os);
    return os;
}

void ArrayRepr::write(std::ostream& os) const {
    os << '[';
    String sep = "";
    auto end = vec_.end();
    auto it = vec_.begin();
    while (it != end) {
        os << sep;
        (*it).write(os);
        sep = ",";
        ++it;
    }
    os << ']';
}

void ObjectRepr::write(std::ostream& os) const {
    os << '{';
    String sep = "";
    for (auto item : vec_) {
        os << sep;
        JSON key(item.key_);
        key.write(os);
        os << ':';
        auto const& v = item.val_;
        v.write(os);
        sep = ",";
    }
    os << '}';
}

cxx::String JSON::str() const {
    std::stringstream ss;
    write(ss);
    return ss.str();
}

}  // namespace cxx

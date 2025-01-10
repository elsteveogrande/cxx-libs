#pragma once
static_assert(__cplusplus >= 202300L, "cxx-libs requires C++23");
// (c) 2024 Steve O'Brien -- MIT License

#include "_JSONBase.h"
#include "_LinkedList.h"

#include <cassert>
#include <cstddef>
#include <cxx/Generator.h>
#include <cxx/String.h>
#include <iostream>

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

}  // namespace cxx

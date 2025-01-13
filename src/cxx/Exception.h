#pragma once
static_assert(__cplusplus >= 202300L, "cxx-libs requires C++23");
// (c) 2024 Steve O'Brien -- MIT License

#include "exc/Exception.h"
#include "stack/Resolver.h"
#include "stack/Trace.h"

#include <iostream>

namespace cxx {

// Declare types here so IDE considers this file (not a decl/ file) "authoritative"
template <class E>
struct Exception;

template <class E>
void Exception<E>::dump(std::ostream& os) const {
    cxx::StackResolver sr;
    trace_.resolve(sr);
    trace_.dump(os);
}

}  // namespace cxx

#include <cxx/StackTrace.h>

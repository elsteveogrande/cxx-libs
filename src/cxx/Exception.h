#pragma once
static_assert(__cplusplus >= 202300L, "cxx-libs requires C++23");
// (c) 2024 Steve O'Brien -- MIT License

#include "exc/Exception.h"
#include "stacktrace/Resolver.h"
#include "stacktrace/Trace.h"

#include <iostream>

namespace cxx {

// Declare types here so IDE considers this file (not a decl/ file) "authoritative"
struct Exception;

void Exception::dump(std::ostream& os) const {
    cxx::StackResolver sr;
    trace->resolve(sr);
    trace->dump(os);
}

}  // namespace cxx

#include <cxx/StackTrace.h>

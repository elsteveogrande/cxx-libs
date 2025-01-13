#pragma once
#include <cstdlib>
#include <utility>
static_assert(__cplusplus >= 202300L, "cxx-libs requires C++23");
// (c) 2024 Steve O'Brien -- MIT License

#include "../ref/Ref.h"
#include "../stack/Trace.h"

#include <cstddef>
#include <cxxabi.h>
#include <exception>
#include <iostream>
#include <sstream>
#include <string>

namespace cxx {

struct StackTrace;

struct ExceptionBase : std::exception {
    virtual ~ExceptionBase() = default;
    virtual void dump(std::ostream& os = std::cerr) const = 0;
};

template <class E>
struct Exception : ExceptionBase {
    StackTrace trace_;
    Ref<std::stringstream> buffer_;
    std::string mutable what_;

    ~Exception() override = default;
    Exception();

    char const* what() const noexcept override {
        what_ = buffer_->str();
        return what_.data();
    }

    void dump(std::ostream& os = std::cerr) const override;

    E const& operator<<(this E const& self, auto&& val) {
        *self.buffer_ << val;
        return self;
    }
};

namespace detail {

void dump(std::exception_ptr eptr) {
    if (!eptr) { return; }
    try {
        std::rethrow_exception(eptr);
    } catch (std::exception const& e) {
        auto excClass = demangle(typeid(e).name());
        std::cerr
                << std::endl
                << "###################################" << std::endl
                << "Uncaught exception:" << std::endl
                << "type: " << excClass << std::endl
                << "what: " << e.what() << std::endl
                << "###################################" << std::endl;
        auto const* ex = dynamic_cast<ExceptionBase const*>(&e);
        if (ex) { ex->dump(); }
    }
}

[[gnu::noreturn]] void uncaughtHandler();

std::terminate_handler ensureUncaughtHandler() {
    static auto prevHandler = std::set_terminate(uncaughtHandler);
    return prevHandler;
}

[[gnu::noreturn]] void uncaughtHandler() {
    dump(std::current_exception());  // dump w/ stacktreace
    ensureUncaughtHandler();         // returns previous handler
    std::unreachable();              // which should have aborted/exited
}

}  // namespace detail

template <class E>
Exception<E>::Exception()
        : ExceptionBase()
        , buffer_(Ref<std::stringstream>::make()) {
    detail::ensureUncaughtHandler();
};

}  // namespace cxx

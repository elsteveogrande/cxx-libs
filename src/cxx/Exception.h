#pragma once
static_assert(__cplusplus >= 202300L, "cxx-libs requires C++23");
// (c) 2024 Steve O'Brien -- MIT License

#include "StackTrace.h"

#include <cstddef>
#include <cxxabi.h>
#include <exception>
#include <iostream>
#include <string>
#include <utility>

namespace cxx {

struct Exception : std::exception {
    cxx::StackTrace trace {};
    std::string const message;

    [[nodiscard]] [[gnu::noinline]] explicit Exception();
    [[nodiscard]] [[gnu::noinline]] explicit Exception(std::string message);

    char const* what() const noexcept override { return message.data(); }
};

namespace detail {

std::string demangle(char const* name) noexcept {
    int st = 0;
    auto* demangled = abi::__cxa_demangle(name, nullptr, nullptr, &st);
    return {demangled};
}

void dump(std::exception_ptr eptr) noexcept {
    std::exception const* e;
    try {
        std::rethrow_exception(eptr);
    } catch (std::exception const& _e) { e = &_e; }

    if (e) {
        std::cerr
                << std::endl
                << "###################################" << std::endl
                << "Uncaught exception:" << std::endl
                << "type: " << demangle(typeid(*e).name()) << std::endl
                << "what: " << e->what() << std::endl
                << "###################################" << std::endl;
        auto const* ex = dynamic_cast<cxx::Exception const*>(e);
        if (ex) { ex->trace.dump(); }
    }
}

void uncaughtHandler();

std::terminate_handler ensureUncaughtHandler() {
    static auto prevHandler = std::set_terminate(uncaughtHandler);
    return prevHandler;
}

void uncaughtHandler() { dump(std::current_exception()); }

}  // namespace detail

[[nodiscard]] [[gnu::noinline]]
Exception::Exception()
        : std::exception() {
    detail::ensureUncaughtHandler();
};

[[nodiscard]] [[gnu::noinline]]
Exception::Exception(std::string message)
        : std::exception()
        , message(std::move(message)) {
    detail::ensureUncaughtHandler();
};

}  // namespace cxx

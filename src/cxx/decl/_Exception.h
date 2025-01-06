#pragma once
static_assert(__cplusplus >= 202300L, "cxx-libs requires C++23");
// (c) 2024 Steve O'Brien -- MIT License

#include <cstddef>
#include <cxxabi.h>
#include <exception>
#include <iostream>
#include <string>
#include <utility>

namespace cxx {

struct StackTrace;

struct Exception : std::exception {
    cxx::StackTrace* trace;
    std::string const message;

    explicit Exception();
    explicit Exception(std::string message);

    char const* what() const noexcept override { return message.data(); }
    void dump() const;
};

namespace detail {

void dump(std::exception_ptr eptr) {
    std::exception const* e;
    try {
        std::rethrow_exception(eptr);
    } catch (std::exception const& _e) { e = &_e; }

    if (e) {
        std::cerr << std::endl
                  << "###################################" << std::endl
                  << "Uncaught exception:" << std::endl
                  << "type: " << typeid(*e).name() << std::endl
                  << "what: " << e->what() << std::endl
                  << "###################################" << std::endl;
        auto const* ex = dynamic_cast<cxx::Exception const*>(e);
        if (ex) { ex->dump(); }
    }
}

void uncaughtHandler();

std::terminate_handler ensureUncaughtHandler() {
    static auto prevHandler = std::set_terminate(uncaughtHandler);
    return prevHandler;
}

void uncaughtHandler() { dump(std::current_exception()); }

}  // namespace detail

[[gnu::noinline]]
Exception::Exception()
        : std::exception() {
    detail::ensureUncaughtHandler();
};

[[gnu::noinline]]
Exception::Exception(std::string message)
        : std::exception()
        , message(std::move(message)) {
    detail::ensureUncaughtHandler();
};

}  // namespace cxx

#pragma once

#include <source_location>
#include <sstream>
#include <type_traits>
#include <utility>

template <typename F>
struct privDefer {
    F f;
    privDefer(F f) : f(f) {}
    ~privDefer() { f(); }
};

template <typename F>
privDefer<F> defer_func(F f) {
    return privDefer<F>(f);
}

#define DEFER_1(x, y) x##y
#define DEFER_2(x, y) DEFER_1(x, y)
#define DEFER_3(x) DEFER_2(x, __COUNTER__)
#define defer(code) auto DEFER_3(_defer_) = defer_func([&]() { code; })

#define LOOM_ASSERT(cond_expr, description)                                              \
    do {                                                                                 \
        if (!(cond_expr)) [[unlikely]] {                                                 \
            std::source_location loc = std::source_location::current();                  \
            std::stringstream ss;                                                        \
            ss << "panic in function: '" << loc.function_name() << "'\n"                 \
               << loc.file_name() << ":" << loc.line() << ":" << loc.column() << ": "    \
               << description << ", triggered by expression: \n"                         \
               << "\t" << loc.line() << "\t|\t ... " << #cond_expr << " ...\n";          \
            std::printf("%s", ss.str().c_str());                                         \
            std::exit(EXIT_FAILURE);                                                     \
        }                                                                                \
    } while (0)

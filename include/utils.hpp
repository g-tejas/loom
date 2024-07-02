#pragma once

#include "quill/LogMacros.h"
#include <format>
#include <source_location>
#include <type_traits>
#include <utility>

template <typename Fx>
struct defer_t {
  Fx _fx;

  explicit defer_t(Fx &&_arg_fx) noexcept(::std::is_nothrow_move_constructible_v<Fx>)
      : _fx(::std::move(_arg_fx)) {}

  ~defer_t() noexcept(::std::is_nothrow_invocable_v<Fx>) { _fx(); }
};

template <typename Fx>
defer_t(Fx _fx) -> defer_t<::std::decay_t<Fx>>;

#define DEFER_TOK_PASTE(X, Y) X##Y
#define defer __defer_t DEFER_TOK_PASTE(__scoped_defer_obj, __COUNTER__) = [&]()

#include <sstream>

#define CHECK_EX(cond_expr, description)                                                 \
  do {                                                                                   \
    if (!(cond_expr)) [[unlikely]] {                                                     \
      std::source_location loc = std::source_location::current();                        \
      std::stringstream ss;                                                              \
      ss << "panic in function: '" << loc.function_name() << "'\n"                       \
         << loc.file_name() << ":" << loc.line() << ":" << loc.column() << ": "          \
         << description << ", triggered by expression: \n"                               \
         << "\t" << loc.line() << "\t|\t ... " << #cond_expr << " ...\n";                \
      std::printf("%s", ss.str().c_str());                                               \
    }                                                                                    \
  } while (0)
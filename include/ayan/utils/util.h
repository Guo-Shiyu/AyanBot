#pragma once

#include "libhv/base/htime.h"

#include <chrono>
#include <thread>

namespace ayan {

using never_t = void;
using namespace std::chrono_literals;

namespace util {
template <typename Time = decltype(1ms)>
never_t block_here(Time &&wake_interval = 5s) {
  while (true) {
    std::this_thread::sleep_for(wake_interval);
  }
}

// /// e.g:
// /// auto ms = count_ms(1min);
// /// assert(ms == 60 * 1000);
// template <class _Rep, class _Period>
// auto count_in_ms(const std::chrono::duration<_Rep, _Period> &real_time)
// {
//     return std::chrono::duration_cast<decltype(1ms)>(real_time).count();
// }

// template <typename... T>
// int accumulate_in_ms(T &&...real_time)
// {
//     return ((count_ms(real_time)) + ...);
// }

inline void clear_screen() noexcept {
  constexpr const char *cmd =
#ifdef _WIN32
      "chcp 65001 & cls";
#else
      "clear";
#endif
  std::system(cmd);
}

// get type's name as string_view in compile time
template <typename T> constexpr auto type_name() noexcept {
  std::string_view name, prefix, suffix;
#ifdef __clang__
  name = __PRETTY_FUNCTION__;
  prefix = "auto type_name() [T = ";
  suffix = "]";
#elif defined(__GNUC__)
  name = __PRETTY_FUNCTION__;
  prefix = "constexpr auto type_name() [with T = ";
  suffix = "]";
#elif defined(_MSC_VER)
  name = __FUNCSIG__;
  prefix = "auto __cdecl type_name<";
  suffix = ">(void) noexcept";
#endif
  name.remove_prefix(prefix.size());
  name.remove_suffix(suffix.size());
  return name;
}

  inline std::string_view time_now()
  {
    static char buf[DATETIME_FMT_BUFLEN];
    datetime_t dt = datetime_now();
    datetime_fmt(&dt, buf);
    return buf;
  }

} // namespace util
} // namespace ayan
#pragma once

#include "ayan/import/fmt.h"
#include "ayan/import/srcloc.h"

#include "ayan/utils/util.h"

#include <iostream>

namespace ayan {

using LogStreamPtr = std::ostream *;
struct LogRedirectSet {
  LogStreamPtr dbg_, log_, err_;

  explicit LogRedirectSet() {
    dbg_ = &std::cout;
    log_ = &std::cout;
    err_ = &std::cerr;
  }

  explicit LogRedirectSet(const LogRedirectSet &) = default;
};

/// 给每一条日志加上 时间, 等级 的前缀
class Logger : protected LogRedirectSet {
public:
  // <time> <level> <custom data>
  constexpr static const char *kLogFmt = "{} {:5} _ {}";

  // <time> <level> <source info> <custom data>
  constexpr static const char *kLogWithLocFmt = "{} {:5} {} {}";

  struct Color {
    constexpr static std::string_view //
        Green = "\033[1;32m",         //
        Red   = "\033[1;31m",         //
        White = "\033[0m";            //
  };

  struct LogLevel {
    constexpr static std::string_view //
        Info  = "INFO",               //
        Debug = "DEBUG",              //
        Error = "ERROR";              //
  };

public:
  Logger()               = default;
  Logger(const Logger &) = default;
  Logger(const LogRedirectSet &lrs) : LogRedirectSet(lrs) {}

  const Logger &log(std::string_view content) const {
    return puts(Color::White, log_, false)
        .puts(fmt::format(kLogFmt, util::time_now(), LogLevel::Info, content), log_);
  }

  const Logger &log(Srcloc loc, std::string_view content) const {
    return puts(Color::White, log_, false)
        .puts(
            fmt::format(
                kLogWithLocFmt, util::time_now(), LogLevel::Info, fmt_srcloc(&loc),
                content),
            log_);
  }

  const Logger &dbg(std::string_view content) const {
    return puts(Color::Green, dbg_, false)
        .puts(fmt::format(kLogFmt, util::time_now(), LogLevel::Debug, content), dbg_);
  }

  const Logger &dbg(Srcloc loc, std::string_view content) const {
    return puts(Color::Green, dbg_, false)
        .puts(
            fmt::format(
                kLogWithLocFmt, util::time_now(), LogLevel::Debug, fmt_srcloc(&loc),
                content),
            dbg_);
  }

  const Logger &err(std::string_view content) const {
    return puts(Color::Red, err_, false)
        .puts(fmt::format(kLogFmt, util::time_now(), LogLevel::Error, content), err_);
  }

  const Logger &err(Srcloc loc, std::string_view content) const {
    return puts(Color::Red, err_, false)
        .puts(
            fmt::format(
                kLogWithLocFmt, util::time_now(), LogLevel::Error, fmt_srcloc(&loc),
                content),
            err_);
  }

private:
  const Logger &
  puts(std::string_view content, std::ostream *os, bool single_line = true) const {
    *os << content;
    if (single_line) {
      *os << std::endl;
    }
    return *this;
  }

  static std::string fmt_srcloc(Srcloc *loc) {
    return fmt::format(
        "{0}:{2:<3} {1}()", loc->file_name(), loc->function_name(), loc->line());
  }
};
} // namespace ayan
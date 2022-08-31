#pragma once

#include "ayan/config.h"
#include <iostream>

namespace ayan
{
    struct LogRedirectSet
    {
        std::ostream *dbg_;
        std::ostream *log_;
        std::ostream *err_;

        explicit LogRedirectSet()
        {
            dbg_ = &std::cout;
            log_ = &std::cout;
            err_ = &std::cerr;
        }

        explicit LogRedirectSet(const LogRedirectSet &) = default;
    };

    struct Logger : protected LogRedirectSet
    {
        template <typename... Args>
        Logger &log(std::string_view fmt, Args &&...args)
        {
            (*log_ << fmt << '\n').flush();
            return *this;
        }

        /// TODO:
        /// err, dbg
    };
}
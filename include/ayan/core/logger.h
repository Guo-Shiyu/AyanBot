#pragma once

#include "ayan/config.h"
#include <iosfwd>

namespace ayan
{
    struct LogRedirectSet
    {
        std::ostream *dbg;
        std::ostream *log;
        std::ostream *err;

        explicit LogRedirectSet();
        explicit LogRedirectSet(const LogRedirectSet &) = default;
    };

    struct Logger : protected LogRedirectSet
    {
        template <typename... Args>
        Logger &log(std::string_view fmt, Args &&...args)
        {
            *log << fmt << std::endl;
            return *this;
        }

        /// TODO:
        /// err, dbg 
    };
}
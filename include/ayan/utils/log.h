#pragma once

#include "source_location/source_location.hpp"
#include "fmt/format.h"

#include <iosfwd>

namespace ayan
{

}

namespace ayan::log
{
    struct RedirectInfo
    {
        std::ostream *log, *err, *dbg;

        /// NOTICE:
        /// default redirect info are defined in
        /// .log = &std::cout,
        /// .err = &std::cerr,
        /// .dbg = &std::cout,
        static RedirectInfo Default();
    };
}
#include "ayan/core/logger.h"

#include <iostream>

namespace ayan
{
    LogRedirectSet::LogRedirectSet() {
        dbg = &std::cout;
        log = &std::cout;
        err = &std::cerr;
    }
}

namespace ayan
{

}

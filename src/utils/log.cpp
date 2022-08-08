#include "ayan/utils/log.h"

#include <iostream>

namespace ayan 
{
    
}


namespace ayan::log
{
    RedirectInfo RedirectInfo::Default()
    {
        return RedirectInfo{
            .log = &std::cout,
            .err = &std::cerr,
            .dbg = &std::cout,
        };
    }
}
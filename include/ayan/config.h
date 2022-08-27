#pragma once

#include <string_view>

namespace ayan
{
    namespace config
    {
        constexpr std::string_view
            kDefaultBotName = "ayan";

        constexpr std::string_view
            kLogFormat = "[{0}] [{1}] [{2}] [{3}]: {4}";
    }
}

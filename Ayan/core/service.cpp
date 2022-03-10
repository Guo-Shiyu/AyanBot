#include "service.h"

namespace ayan
{
    namespace default_service
    {
        std::string_view KeepAlive::name() const
        {
            return SelfName;
        }

        bool KeepAlive::handle(Bot &bot, MetaEvent &meta)
        {
            return true;
        }

    }
}
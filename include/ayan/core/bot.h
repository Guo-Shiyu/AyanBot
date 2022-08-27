#pragma once

#include "ayan/config.h"
#include "ayan/core/logger.h"
#include "ayan/import/optional.h"
#include "ayan/import/websocket.h"
#include "ayan/service/service.h"

#include <string>
#include <memory>

namespace ayan
{
    class Bot : std::enable_shared_from_this<Bot>
    {
    public:
    private:
        Bot() = delete;

    public:
        explicit Bot();

    private:
        ServiceManager sev_;
        Logger loggger_;
        WebSocketClient ws_;
    };

    class BotBuilder
    {
    public:
        using Self = BotBuilder;

    private:
        BotBuilder() = default;

    public:
        BotBuilder(const BotBuilder &) = delete;

    private:
        Optional<std::string> botname_; // bot's name
        std::string netaddr_;           // onebot server addr
        LogRedirectSet logset_;         // log redirect set

    public:
        Self &connect(std::string_view ip, int port);
        Self &connect(std::string_view url); // "ws://127.0.0.1:6700/"

        Self &set_dbg(std::ostream *os)
        {
            logset_.dbg = os;
            return *this;
        }

        Self &set_log(std::ostream *os)
        {
            logset_.log = os;
            return *this;
        }

        Self &set_err(std::ostream *os)
        {
            logset_.err = os;
            return *this;
        }

        Bot build();

        static BotBuilder create();
        static BotBuilder with_name(std::string_view name);
    };
}

namespace ayan
{

}

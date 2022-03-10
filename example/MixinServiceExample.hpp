#pragma once

#include "../Ayan/core/service.h"
#include "../Ayan/core/apicall.h"

/*
 *  unstable example
 *
 *  do not use these classes
 */

struct MsgAndReqService : public MixinService<MessageService<MsgAndReqService>, RequestService<MsgAndReqService>>
{
    virtual ~MsgAndReqService() = default;

    std::string_view name() const override
    {
        return "msg and req service test";
    }
};

struct MsgAndMetaService : public MixinService<MessageService<MsgAndMetaService>, RequestService<MsgAndMetaService>>
{
    virtual ~MsgAndMetaService() = default;

    std::string_view name() const override
    {
        if constexpr (std::is_base_of<ServiceBase<MsgAndMetaService>, MsgAndMetaService>())
        {
            int x = 0;
        }
        return "msg and meta service test";
    }
};
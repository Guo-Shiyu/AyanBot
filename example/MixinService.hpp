#pragma once

#include "../core/service.h"
#include "../core/apicall.h"

struct MsgAndReqService : public MixinService<MessageService<MsgAndReqService>, RequestService<MsgAndReqService>>
{
    ~MsgAndReqService() = default;

    std::string_view name() const override
    {
        return "msg and req service";
    }

	bool trig(Bot& bot, RequestEvent& req)
	{
		return false;
	}

	void act(Bot& bot, RequestEvent& req)
	{

	}

	bool trig(Bot& bot, PrivateMessage& pri)
	{
		return false;
	}

	void act(Bot& bot, PrivateMessage& pri)
	{

	}

	bool trig(Bot& bot, GroupMessage& gro)
	{
		return false;
	}

	void act(Bot& bot, GroupMessage& gro)
	{

	}
};

struct MsgAndMetaService : public MixinService<MessageService<MsgAndMetaService>, RequestService<MsgAndMetaService>>
{
    virtual ~MsgAndMetaService() = default;

    std::string_view name() const override
    {
        if constexpr (std::is_base_of<ServiceBase<MsgAndMetaService>, MsgAndMetaService>())
        {
            std::exit(137);
        }
        return "msg and meta service test";
    }
};
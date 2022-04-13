#pragma once

#include "../core/service.h"
#include "../core/apicall.h"

class PermitReq : public RequestService<PermitReq>
{
public:
	std::string_view name() const override
	{
		return "Permit Request";
	}

	void on_load(Bot &bot)
	{
		bot.dbg("I will permit all requests.");
	}

	bool trig(Bot &bot, RequestEvent &req)
	{
		return true;
	}

	void act(Bot &bot, RequestEvent &req)
	{
		auto bot_ptr = bot.shared_from_this();
		Overloaded match{
			[bot_ptr](const FriendRequest &freq)
			{
				bot_ptr->api()
					.set_friend_add_request(freq.flag, true, "") // 同意并且不设置备注
					.get();
			},

			[bot_ptr](const GroupRequest &greq)
			{
				bot_ptr->api()
					.set_group_add_request(greq.flag, enum_to_str(greq.type), true)
					.get();
			}};

		std::visit(match, req);
	}

private:
	static constexpr auto 
	enum_to_str = [](GroupReqSubType et) -> const char *
	{
		switch (et)
		{
		case GroupReqSubType::Add:
			return "add";
			break;
		case GroupReqSubType::Invite:
			return "invite";
			break;

		default: // unreachable code
			return "";
			break;
		}
	};
};
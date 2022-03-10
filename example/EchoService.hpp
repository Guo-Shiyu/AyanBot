#pragma once

#include "../Ayan/core/service.h"
#include "../Ayan/core/apicall.h"

class Echo : public MessageService<Echo>
{
public:
	std::string_view name() const override
	{
		return "Echo Service";
	}

	bool trig(Bot& bot, PrivateMessage& msg)
	{
		return true;
	}

	bool trig(Bot& bot, GroupMessage& msg)
	{
		return true;
	}

	void act(Bot& bot, PrivateMessage& msg)
	{
		bot.api().send_private_msg({ msg.sender.user_id, msg.message });
	}

	void act(Bot& bot, GroupMessage& msg)
	{
		bot.api().send_group_msg({ msg.group_id, msg.message });
	}
};


#pragma once

#include "../core/service.h"
#include "../core/apicall.h"

class Hello : public MessageService<Hello>
{
public:
	std::string_view name() const override
	{
		return "Hello Service";
	}

	void on_load(Bot& bot)
	{
		std::string_view now = time_now();
		Message hello = std::string{ now };

		bot.log("Welcome!, now is {0}", now);
		
		for (Qid super : bot.admins())
		{
			bot.api().send_private_msg(super, hello);
		}
	}
};

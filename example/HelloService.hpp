#pragma once

#include "../core/service.h"
#include "../core/apicall.h"

class HelloService : public MetaService<HelloService>
{
public:
	std::string_view name() const override
	{
		return "Hello-Service";
	}

	void on_load(Bot& bot)
	{
		std::string_view now_view = time_now();
		Message hello =  std::string{ now_view };

		bot.log("~ Hello Service ~ ");
		
		for (Qid super : bot.admins())
		{
			bot.api().send_private_msg({ super, hello });
		}
	}
};

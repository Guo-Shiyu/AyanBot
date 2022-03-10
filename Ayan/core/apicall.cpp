#include "env.h"
#include "apicall.h"

namespace ayan
{
	std::default_random_engine ApiCall::random{};

	ApiCall::ApiCall(const BotPtr &bot)
		: _packet({}), _bot(bot)
	{
	}

	ApiCall ApiCall::from(const BotPtr &bot)
	{
		return ApiCall(bot);
	}

	ApiCall &ApiCall::action(std::string_view api_name)
	{
		_packet["action"] = api_name;
		return *this;
	}

	ApiCall &ApiCall::clear_packet()
	{
		_packet.clear();
		return *this;
	}

	ApiCall &ApiCall::remove_param(std::string_view key)
	{
		_packet.erase(key.data());
		return *this;
	}

	void ApiCall::call(size_t echo_code)
	{
		echo(echo_code);
		_bot->channel()
			->send(_packet.dump());
	}

	ApiWarpper &ApiCall::api()
	{
		return *reinterpret_cast<ApiWarpper *>(this);
	}
}
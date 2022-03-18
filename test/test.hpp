#include "../Ayan/core/apicall.h"

using namespace ayan;
using namespace std::chrono_literals;

void raw_api_call_test(BotPtr& bot)
{
	auto msg = MsgBuilder::from(L"raw api call test").build();

	bot->active()
		.action("send_private_msg")
		.params("user_id", 2821006329)
		.params("message", msg.as_json())
		.call();
}

void future_api_get_test(BotPtr& bot)
{
	auto msg = MsgBuilder::from(L"future api rest: get").build();

	auto unpacker = [](const auto& _)
	{
		return 42;
	};

	auto some = bot->active()
		.action("send_private_msg")
		.params("user_id", 2821006329)
		.params("message", msg.as_json())
		.get<int>(unpacker);

	std::cout << "future:" << some.get() << std::endl;
}

void callback_api_handle_test(BotPtr& bot)
{
	auto msg = MsgBuilder::from(L"callback api test: handle").build();

	using ComplexType = std::tuple<int, int, std::string>;
	auto unpacker = [](const json& _) -> ComplexType
	{
		return std::make_tuple(12, 14, std::string("abc"));
	};

	auto expect = [](ComplexType&& tup) -> void
	{
		auto& [x, y, z] = tup;
		std::cout << x << ' ' << y << ' ' << z << std::endl;
	};

	bot->active()
		.action("send_private_msg")
		.params("user_id", (Qid)2821006329)
		.params("message", msg.as_json())
		.handle<ComplexType>(unpacker, expect);
}

void api_call_test(BotPtr& bot)
{

	Message msg = Message(L"1234");

	MsgId id = bot->api()
		.send_private_msg(2821006329, msg)
		.get();

	std::cout << "private-call:" << id << std::endl;

	id = bot->api()
		.send_group_msg(933327998, msg)
		.get();

	std::cout << "public-call:" << id << std::endl;

}

void msg_recall_test(BotPtr& bot)
{
	Message msg = Message(__FUNCSIG__);
	//Message msg = Message("try_again");

	auto fu_id = bot->api().send_group_msg( 933327998, msg );

	auto id = fu_id.get();

	std::this_thread::sleep_for(5s);

	bot->api().delete_msg(id);
}

void at_and_reply_test(BotPtr& bot)
{
	auto fst = Message(u8"some msg");

	auto msgid = bot->api().send_group_msg(933327998, fst).get();

	auto scd = MsgBuilder::from(L"hahaha")
		.at(2821006329)
		.face(55)
		.reply(msgid)
		.build();

	bot->api().send_group_msg({ 933327998, scd });
}

void image_send_test(BotPtr& bot)
{
	std::string image_path = R"(C:\Users\User\Pictures\3.jpg)";
	std::string baidu_image_url = R"(https://gimg2.baidu.com/image_search/src=http%3A%2F%2Fimg.zcool.cn%2Fcommunity%2F01292755455f520000019ae9ec349b.jpg%401280w_1l_2o_100sh.jpg&refer=http%3A%2F%2Fimg.zcool.cn&app=2002&size=f9999,10000&q=a80&n=0&g=0n&fmt=jpeg?sec=1649147570&t=99523a88550a4e4a4963e43580b1de22)";

	auto thd = MsgBuilder::from()
		//.image_local(image_path)
		.image_url(baidu_image_url, false, false, false)
		.build();

	bot->api().send_group_msg({ 933327998, thd });
}

void get_friend_list_test(BotPtr& bot)
{
	auto friend_list = bot->api().get_friend_list();

	std::this_thread::sleep_for(3s);
	auto vec = friend_list.get();
}

void get_group_info_test(BotPtr& bot)
{
	auto group_info = bot->api().get_group_info(933327998).get();
}

void get_group_list_test(BotPtr& bot)
{
	auto group_list = bot->api().get_group_list().get();
}
#include "event.h"
#include "utility.h"

#include <mutex>

#pragma warning(disable : 4715)

/// message event
namespace ayan
{

	PrivateMessage MessageEventParser::parse_private(const json &packet)
	{
		auto parse_subtype = [](const std::string_view &raw) -> PrivateSubType
		{
			if (raw.starts_with('f'))
				return PrivateSubType::Friend;
			else //if (raw.starts_with('g'))
				return PrivateSubType::Group;
			// else
			// 	unreachable();
		};

		auto parse_sender = [](const json &sender_field) -> PrivateMessageSender
		{
			const auto &sender = sender_field;
			return PrivateMessageSender{
				.user_id = sender["user_id"].get<Qid>(),
				.nickname = utf8_to_wstr(sender["nickname"].get<std::string>()),
				.sex = utf8_to_wstr(sender["sex"].get<std::string>()),
				.age = sender["age"].get<int>(),
			};
		};

		return PrivateMessage{
			.subtype = parse_subtype(packet["sub_type"].get<std::string_view>()),
			.sender = parse_sender(packet["sender"]),
			.time = packet["time"].get<int64_t>(),
			.msgid = packet["message_id"].get<int>(),
			.message = parse_message(packet["message"]),
		};
	}

	GroupMessage MessageEventParser::parse_group(const json &packet)
	{
		auto parse_subtype = [](std::string_view raw) -> GroupMsgSubType
		{
			static std::map<std::string_view, GroupMsgSubType> stmap{};
			static std::once_flag once{};
			std::call_once(once, []() -> void
						   {
					stmap.emplace("normal",		GroupMsgSubType::Normal);
					stmap.emplace("anonymous",	GroupMsgSubType::Anonymous);
					stmap.emplace("notice",		GroupMsgSubType::Notice); });
			return stmap.at(raw);
		};

		auto parse_sender = [](const json &sender_field) -> GroupMessageSender
		{
			auto parse_role = [](const std::string_view &raw) -> GroupRole
			{
				static std::map<std::string_view, GroupRole> stmap;
				static std::once_flag once;
				std::call_once(once, []() -> void
							   {
						stmap.emplace("admin", GroupRole::Admin);
						stmap.emplace("member", GroupRole::Member);
						stmap.emplace("owner", GroupRole::Owner); });
				return stmap.at(raw);
			};

			const auto &sender = sender_field;

			GroupMessageSender ret{{
				.user_id = sender["user_id"].get<Qid>(),
				.nickname = utf8_to_wstr(sender["nickname"].get<std::string>()),
				.sex = utf8_to_wstr(sender["sex"].get<std::string>()),
				.age = sender["age"].get<int>(),
			}};

			ret.card = utf8_to_wstr(sender["card"].get<std::string>());
			ret.area = utf8_to_wstr(sender["area"].get<std::string>());
			ret.level = utf8_to_wstr(sender["level"].get<std::string>());
			ret.title = utf8_to_wstr(sender["title"].get<std::string>());
			ret.role = parse_role(sender["role"].get<std::string_view>());
			return ret;
		};

		GroupMessage ret{{.subtype = parse_subtype(packet["sub_type"].get<std::string_view>()),
						  .sender = parse_sender(packet["sender"]),
						  .time = packet["time"].get<int64_t>(),
						  .msgid = packet["message_id"].get<int>(),
						  .message = parse_message(packet["message"])}};
		ret.group_id = packet["group_id"].get<Qid>();
		return ret;
	}

	Message MessageEventParser::parse_message(const json &msg_field)
	{
		return Message::from(msg_field);
	}
}

/// meta event
namespace ayan
{
	HeartBeat MetaEventParser::parse_heartbeat(const json &packet)
	{
		return HeartBeat{
			.time = packet["time"].get<int64_t>(),
			.interval = packet["interval"].get<int64_t>(),
			// .status = parse_status(packet["status"]),
		};
	}

	Status MetaEventParser::parse_status(const json &packet)
	{
		return Status{
			.good = packet["good"].get<bool>(),
			.statistics = parse_statistic(packet["stat"]),
		};
	}

	Statistic MetaEventParser::parse_statistic(const json &packet)
	{
		return Statistic{
			.pk_recv = packet["packet_received"].get<size_t>(),
			.pk_send = packet["packet_sent"].get<size_t>(),
			.msg_recv = packet["message_received"].get<size_t>(),
			.msg_send = packet["packet_sent"].get<size_t>(),
			.pk_lost = packet["packet_lost"].get<unsigned>(),
			.tcp_disc_times = packet["disconnect_times"].get<unsigned>(),
			.drop_times = packet["lost_times"].get<unsigned>(),
		};
	}

	LifeCycle MetaEventParser::parse_lifecycle(const json &packet)
	{
		return LifeCycle{
			.time = packet["time"],
			.subtype = LifeCycSubType::Connect,
		};
	}
}

/// notice event
namespace ayan
{
	GroupMemberLeave NoticeEventParser::parse_group_leave(const json &packet)
	{
		auto parse_type = [](const std::string_view &type) -> LeaveType
		{
			if (type.starts_with('l'))
				return LeaveType::Leave;
			else if (type.ends_with('k'))
				return LeaveType::Kick;
			else
				return LeaveType::Kick_me;
		};

		return GroupMemberLeave{
			.type = parse_type(packet["sub_type"].get<std::string_view>()),
			.group_id = packet["group_id"].get<Qid>(),
			.operator_id = packet["operator_id"].get<Qid>(),
			.user_id = packet["user_id"].get<Qid>(),
		};
	}

	GroupMemberJoin NoticeEventParser::parse_group_join(const json &packet)
	{
		auto parse_type = [](const std::string_view &type) -> JoinType
		{
			if (type.starts_with('a'))
				return JoinType::Approve;
			else
				return JoinType::Invite;
		};

		return GroupMemberJoin{
			.type = parse_type(packet["sub_type"].get<std::string_view>()),
			.group_id = packet["group_id"].get<Qid>(),
			.operator_id = packet["operator_id"].get<Qid>(),
			.user_id = packet["user_id"].get<Qid>(),
		};
	}

	FriendMsgRecall NoticeEventParser::parse_friend_recall(const json &packet)
	{
		return FriendMsgRecall{
			.user_id = packet["user_id"].get<Qid>(),
			.msg_id = packet["message_id"].get<MsgId>(),
		};
	}

	GroupMsgRecall NoticeEventParser::parse_group_recall(const json &packet)
	{
		return GroupMsgRecall{
			.group_id = packet["group_id"].get<Qid>(),
			.operator_id = packet["operator_id"].get<Qid>(),
			.msg_id = packet["message_id"].get<MsgId>(),
		};
	}

	GroupPoke NoticeEventParser::parse_group_poke(const json &packet)
	{
		return GroupPoke{
			.group_id = packet["group_id"].get<Qid>(),
			.user_id = packet["user_id"].get<Qid>(),
			.target_id = packet["target_id"].get<Qid>(),
		};
	}

	GroupLuckyKing NoticeEventParser::parse_group_lucky_king(const json &packet)
	{
		return GroupLuckyKing{
			.group_id = packet["group_id"].get<Qid>(),
			.user_id = packet["user_id"].get<Qid>(),
			.king_id = packet["target_id"].get<Qid>(),
		};
	}
	FriendPoke NoticeEventParser::parse_friend_poke(const json &packet)
	{
		return FriendPoke{
			.sender_id = packet["sender_id"].get<Qid>(),
		};
	}
}

/// request event
namespace ayan
{
	FriendRequest RequestEventParser::parse_friend(const json &packet)
	{
		return FriendRequest{
			.user_id = packet["user_id"].get<Qid>(),
			.comment = packet["comment"].get<std::string>(),
			.flag = packet["flag"].get<std::string>(),
		};
	}

	GroupRequest RequestEventParser::parse_group(const json &packet)
	{
		auto parse_type = [](const std::string_view &raw)
		{
			if (raw.starts_with('a'))
				return GroupReqSubType::Add;
			else
				return GroupReqSubType::Invite;
		};

		return GroupRequest{
			.type = parse_type(packet["sub_type"].get<std::string_view>()),
			.group_id = packet["group_id"].get<Qid>(),
			.user_id = packet["user_id"].get<Qid>(),
			.comment = packet["comment"].get<std::string>(),
			.flag = packet["flag"].get<std::string>(),
		};
	}

}

namespace ayan
{
	Event EventParser::parse(const json &packet)
	{
		static std::map<std::string_view, std::function<Event(const json &)>> epmap{};
		static std::once_flag once{};
		std::call_once(once, [&]() -> void
		{
				epmap.emplace("message",	parse_message);
				epmap.emplace("request",	parse_request);
				epmap.emplace("notice",     parse_notice);	
				epmap.emplace("meta_event",	parse_meta); 
		});

		auto post_type = packet["post_type"].get<std::string_view>();
		return epmap.at(post_type)(packet);
	}

	MetaEvent EventParser::parse_meta(const json &packet)
	{
		static std::map<std::string_view, std::function<MetaEvent(const json &)>> mep_map{};
		static std::once_flag once{};
		std::call_once(once, [&]() -> void
		{
				mep_map.emplace("heartbeat",	MetaEventParser::parse_heartbeat);
				mep_map.emplace("lifecycle",	MetaEventParser::parse_lifecycle); 
		});

		auto meta_type = packet["meta_event_type"].get<std::string_view>();
		return mep_map.at(meta_type)(packet);
	}

	NoticeEvent EventParser::parse_notice(const json &packet)
	{
		static std::map<std::string_view, std::function<NoticeEvent(const json &)>> nep_map{};
		static std::once_flag once{};
		std::call_once(once, [&]() -> void
					   {
				nep_map.emplace("group_decrease",	NoticeEventParser::parse_group_leave);
				nep_map.emplace("group_increase",	NoticeEventParser::parse_group_join);
				nep_map.emplace("friend_recall",	NoticeEventParser::parse_friend_recall);
				nep_map.emplace("group_recall",		NoticeEventParser::parse_group_recall);
				nep_map.emplace("lucky_king",		NoticeEventParser::parse_group_lucky_king);
				nep_map.emplace("notify", [](const json& packet) -> NoticeEvent
					{
						if (packet.contains("sender_id"))
							return NoticeEventParser::parse_friend_poke(packet);
						else
							return NoticeEventParser::parse_group_poke(packet);
					}); });

		auto not_type = packet["notice_type"].get<std::string_view>();
		return nep_map.at(not_type)(packet);
	}

	MessageEvent EventParser::parse_message(const json &packet)
	{
		static std::map<std::string_view, std::function<MessageEvent(const json &)>> mep_map{};
		static std::once_flag once{};
		std::call_once(once, [&]() -> void
					   {
				mep_map.emplace("private",	MessageEventParser::parse_private);
				mep_map.emplace("group",	MessageEventParser::parse_group); });

		auto msg_type = packet["message_type"].get<std::string>();
		return mep_map.at(msg_type)(packet);
	}

	RequestEvent EventParser::parse_request(const json &packet)
	{
		static std::map<std::string_view, std::function<RequestEvent(const json &)>> rep_map{};
		static std::once_flag once{};
		std::call_once(once, [&]() -> void
					   {
				rep_map.emplace("friend",	RequestEventParser::parse_friend);
				rep_map.emplace("group",	RequestEventParser::parse_group); });

		auto req_type = packet["request_type"].get<std::string>();
		return rep_map.at(req_type)(packet);
	}

	std::string EventRoughDumper::operator()(MetaEvent &meta)
	{
		Overloaded match{
			[](HeartBeat &) -> const char *
			{
				return "Heart Beat";
			},
			[](LifeCycle &) -> const char *
			{
				return "Life Cycle";
			}};
		return std::string("Meta: ").append(std::visit(match, meta));
	}

	std::string EventRoughDumper::operator()(NoticeEvent &notice)
	{
		Overloaded match{
			[](FriendMsgRecall &)
			{
				return "Friend Message Recall";
			},
			[](GroupMsgRecall &)
			{
				return "Group Message Recall";
			},
			[](GroupMemberJoin &)
			{
				return "Group Member Join";
			},
			[](GroupMemberLeave &)
			{
				return "Group Member Leave";
			},
			[](GroupPoke &)
			{
				return "Group Poke";
			},
			[](FriendPoke &)
			{
				return "Friend Poke";
			},
			[](GroupLuckyKing &)
			{
				return "Group Lucky King";
			}};

		return std::string("Notice: ").append(std::visit(match, notice));
	}

	std::string EventRoughDumper::operator()(MessageEvent &msg)
	{
		Overloaded match{
			[](PrivateMessage &pm)
			{
				return std::string("Private Msg from ").append(std::to_string(pm.sender.user_id));
			},
			[](GroupMessage &gm)
			{
				return std::string("Group Msg from ").append(std::to_string(gm.group_id));
			}};
		return std::string("Message: ").append(std::visit(match, msg));
	}

	std::string EventRoughDumper::operator()(RequestEvent &req)
	{
		Overloaded match{
			[](FriendRequest &fr)
			{
				return std::string("Friend from ").append(std::to_string(fr.user_id));
			},
			[](GroupRequest &gr)
			{
				return std::string("Group from ").append(std::to_string(gr.group_id));
			}};

		return std::string("Request: ").append(std::visit(match, req));
	}
}
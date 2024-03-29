#include "ayan/onebot/serde.h"
#include "ayan/onebot/event.h"
#include "ayan/onebot/message.h"

#include "ayan/utils/container.h"
#include "ayan/utils/util.h"

using namespace onebot;

namespace ayan {

template <typename T> using ParserFn = std::function<T(const json &)>;

template <typename E, size_t Size = std::variant_size_v<E>>
using ParserMap = KDenseMap<std::string_view, ParserFn<E>, Size>;

const static ParserMap<RequestEvent> RequestCata = {
    {"friend",
     [](const json &packet) {
       return FriendRequest{
           .user_id = packet["user_id"].get<Qid>(),
           .comment = utf8::utf8to32(packet["comment"].get<std::string_view>()),
           .flag    = packet["flag"].get<std::string>(),
       };
     }},

    {"group",
     [](const json &packet) {
       auto parse_type = [](const std::string_view &raw) {
         if (raw.starts_with('a'))
           return GroupReqSubType::Add;
         else
           return GroupReqSubType::Invite;
       };

       return GroupRequest{
           .type     = parse_type(packet["sub_type"].get<std::string_view>()),
           .group_id = packet["group_id"].get<Qid>(),
           .user_id  = packet["user_id"].get<Qid>(),
           .comment = utf8::utf8to32(packet["comment"].get<std::string_view>()),
           .flag    = packet["flag"].get<std::string>(),
       };
     }},
};

const static ParserMap<MsgEvent> MsgCata = {
    {"private",
     [](const json &packet) {
       auto parse_subtype = [](const std::string_view &raw) -> PrivateSubType {
         if (raw.starts_with('f'))
           return PrivateSubType::Friend;
         else // if (raw.starts_with('g'))
           return PrivateSubType::Group;
         // else
         // 	unreachable();
       };

       auto parse_sender =
           [](const json &sender_field) -> PrivateMessageSender {
         const auto &sender = sender_field;
         return PrivateMessageSender{
             .user_id = sender["user_id"].get<Qid>(),
             .nickname =
                 utf8::utf8to32(sender["nickname"].get<std::string_view>()),
             .sex = utf8::utf8to32(sender["sex"].get<std::string_view>()),
             .age = sender["age"].get<int>(),
         };
       };

       return PrivateMessage{
           .subtype = parse_subtype(packet["sub_type"].get<std::string_view>()),
           .sender  = parse_sender(packet["sender"]),
           .time    = packet["time"].get<int64_t>(),
           .msgid   = packet["message_id"].get<int>(),
           .message = MessageView{MessageParser::from_raw(packet["message"]),
                                  packet["message_id"]},
       };
     }},

    {"group",
     [](const json &packet) {
       auto parse_subtype = [](std::string_view raw) -> GroupMsgSubType {
         constexpr static KDenseMap<std::string_view, GroupMsgSubType, 3> stmap{
             {{"normal", GroupMsgSubType::Normal},
              {"anonymous", GroupMsgSubType::Anonymous},
              {"notice", GroupMsgSubType::Notice}},
         };
         return stmap.at(raw);
       };

       auto parse_sender = [](const json &sender_field) -> GroupMessageSender {
         auto parse_role = [](const std::string_view &raw) -> GroupRole {
           constexpr static KDenseMap<std::string_view, GroupRole, 3> stmap = {{
               {"admin", GroupRole::Admin},
               {"member", GroupRole::Member},
               {"owner", GroupRole::Owner},
           }};
           return stmap.at(raw);
         };

         const auto &sender = sender_field;

         GroupMessageSender ret{{
             .user_id = sender["user_id"].get<Qid>(),
             .nickname =
                 utf8::utf8to32(sender["nickname"].get<std::string_view>()),
             .sex = utf8::utf8to32(sender["sex"].get<std::string_view>()),
             .age = sender["age"].get<int>(),
         }};

         ret.card  = utf8::utf8to32(sender["card"].get<std::string_view>());
         ret.area  = utf8::utf8to32(sender["area"].get<std::string_view>());
         ret.level = utf8::utf8to32(sender["level"].get<std::string_view>());
         ret.title = utf8::utf8to32(sender["title"].get<std::string_view>());
         ret.role  = parse_role(sender["role"].get<std::string_view>());
         return ret;
       };

       GroupMessage ret{
           {.subtype =
                parse_subtype(packet["sub_type"].get<std::string_view>()),
            .sender  = parse_sender(packet["sender"]),
            .time    = packet["time"].get<int64_t>(),
            .msgid   = packet["message_id"].get<int>(),
            .message = MessageView{MessageParser::from_raw(packet["message"]),
                                   packet["message_id"]}}};
       ret.group_id = packet["group_id"].get<Qid>();
       return ret;
     }},
};

const static ParserMap<NoticeEvent, 6> NoticeCata = {
    {"group_decrease",
     [](const json &packet) {
       auto parse_type = [](const std::string_view &type) -> LeaveType {
         if (type.starts_with('l'))
           return LeaveType::Leave;
         else if (type.ends_with('k'))
           return LeaveType::Kick;
         else
           return LeaveType::KickMe;
       };

       return GroupMemberLeave{
           .type     = parse_type(packet["sub_type"].get<std::string_view>()),
           .group_id = packet["group_id"].get<Qid>(),
           .operator_id = packet["operator_id"].get<Qid>(),
           .user_id     = packet["user_id"].get<Qid>(),
       };
     }},

    {"group_increase",
     [](const json &packet) {
       auto parse_type = [](const std::string_view &type) -> JoinType {
         if (type.starts_with('a'))
           return JoinType::Approve;
         else
           return JoinType::Invite;
       };

       return GroupMemberJoin{
           .type     = parse_type(packet["sub_type"].get<std::string_view>()),
           .group_id = packet["group_id"].get<Qid>(),
           .operator_id = packet["operator_id"].get<Qid>(),
           .user_id     = packet["user_id"].get<Qid>(),
       };
     }},

    {"friend_recall",
     [](const json &packet) {
       return FriendMsgRecall{
           .user_id = packet["user_id"].get<Qid>(),
           .msg_id  = packet["message_id"].get<MsgId>(),
       };
     }},

    {"group_recall",
     [](const json &packet) {
       return GroupMsgRecall{
           .group_id    = packet["group_id"].get<Qid>(),
           .operator_id = packet["operator_id"].get<Qid>(),
           .msg_id      = packet["message_id"].get<MsgId>(),
       };
     }},

    {"lucky_king",
     [](const json &packet) {
       return GroupLuckyKing{
           .group_id = packet["group_id"].get<Qid>(),
           .user_id  = packet["user_id"].get<Qid>(),
           .king_id  = packet["target_id"].get<Qid>(),
       };
     }},
     
    {"notify",
     [](const json &packet) {
       if (packet.contains("sender_id"))
         return [&]() -> NoticeEvent {
           return FriendPoke{
               .sender_id = packet["sender_id"].get<Qid>(),
           };
         }();
       else
         return [&]() -> NoticeEvent {
           return GroupPoke{
               .group_id  = packet["group_id"].get<Qid>(),
               .user_id   = packet["user_id"].get<Qid>(),
               .target_id = packet["target_id"].get<Qid>(),
           };
         }();
     }},
};

const static ParserMap<MetaEvent> MetaCata = {
    {
        "heartbeat",
        [](const json &packet) -> MetaEvent {
          return HeartBeat{
              .time     = packet["time"].get<int64_t>(),
              .interval = packet["interval"].get<int64_t>(),
              // .status = parse_status(packet["status"]),};
          };
        },
    },

    {
        "lifecycle",
        [](const json &packet) -> MetaEvent {
          return LifeCycle{
              /// TODO:
              /// fix subtype here
              .subtype = LifeCycSubType::Connect,
              .time    = packet["time"],
          };
        },
    }};

const static ParserMap<Event> TopCata = {
    {"meta_event",
     [](const json &j) {
       auto meta_type = j["meta_event_type"].get<std::string_view>();
       return std::invoke(MetaCata.at(meta_type), j);
     }},
    {"notice",
     [](const json &j) {
       auto notice_type = j["notice_type"].get<std::string_view>();
       return std::invoke(NoticeCata.at(notice_type), j);
     }},
    {"message",
     [](const json &j) {
       auto msg_type = j["message_type"].get<std::string_view>();
       return std::invoke(MsgCata.at(msg_type), j);
     }},
    {"request",
     [](const json &j) {
       auto req_type = j["request_type"].get<std::string_view>();
       return std::invoke(RequestCata.at(req_type), j);
     }},
};

Event EventParser::from_raw(const json &packet) {
  auto post_type = packet["post_type"].get<std::string_view>();
  return std::invoke(TopCata.at(post_type), packet);
}

} // namespace ayan
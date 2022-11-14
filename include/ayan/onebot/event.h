#pragma once

#include "ayan/onebot/message.h"

/// https://github.com/botuniverse/onebot-11/blob/master/event/message.md

namespace onebot {
namespace message {
namespace detail {
template <typename T, typename S> struct MessageBase {
  T subtype;
  S sender;
  int64_t time;    // 时间戳
  MsgId msgid;     // 消息 id
  Message message; // 消息内容
};
} // namespace detail

struct PrivateMessageSender {
  Qid user_id;             // 发送者 qq 号
  std::u32string nickname; // 昵称
  std::u32string sex;      // 性别  "male" | "female" | "unknown"
  int age;                 // 年龄
};

enum class PrivateSubType {
  Friend,    // 好友消息
  Group,     // 群私聊
  GroupSelf, // TODO
  Other,     // 其他
};

using PrivateMessage =
    detail::MessageBase<PrivateSubType, PrivateMessageSender>;

enum class GroupMsgSubType {
  Normal,    // 普通群消息
  Anonymous, // 匿名群消息
  Notice,    // 系统提示 (例如: 管理员已禁止群内匿名聊天)
};

enum class GroupRole {
  Member, // 普通群成员
  Admin,  // 管理员
  Owner,  // 群主
};

struct GroupMessageSender : public PrivateMessageSender {
  GroupRole role;
  std::u32string card;  // 群名片
  std::u32string area;  // 地区
  std::u32string level; // 等级
  std::u32string title; // 专属头衔
};

struct GroupMessage
    : public detail::MessageBase<GroupMsgSubType, GroupMessageSender> {
  Qid group_id; // 群号
};

using MsgEvent = std::variant<PrivateMessage, GroupMessage>;
} // namespace message

namespace notice {
// 好友消息撤回
struct FriendMsgRecall {
  Qid user_id;  // 好友， 消息的撤回者
  MsgId msg_id; // 撤回的消息 id
};

// 群消息撤回
struct GroupMsgRecall {
  Qid group_id, operator_id; // 群号， 操作者qq号
  MsgId msg_id;              // 撤回的消息 id
};

enum class JoinType {
  Approve, // 管理员同意入群
  Invite,  // 管理员邀请入群
};

// 群聊人数增加
struct GroupMemberJoin {
  JoinType type;   // 入群类型
  Qid group_id;    // 群号
  Qid operator_id; // 执行该动作的管理员 qq 号
  Qid user_id;     // 新用户 qq 号
};

enum class LeaveType {
  Leave,  // 主动退群
  Kick,   // 被踢
  KickMe, // 机器人自身被踢
};

// 群聊人数减少
struct GroupMemberLeave {
  LeaveType type;
  Qid group_id;    // 群号
  Qid operator_id; // 踢人操作者的 qq 号
  Qid user_id;     // 被踢/离开的人 qq 号
};

// 群戳一戳
struct GroupPoke {
  Qid group_id, user_id, target_id; // 群号， 发起者， 被戳者
};

// 好友戳一戳
struct FriendPoke {
  Qid sender_id; // 发起者
};

// 群红包幸运王
struct GroupLuckyKing {
  Qid group_id, user_id, king_id; // 群号， 发红包的人， 幸运王
};

// struct NoticeEventParser
// {
//     static GroupMemberLeave parse_group_leave(const json &packet);
//     static GroupMemberJoin parse_group_join(const json &packet);
//     static FriendMsgRecall parse_friend_recall(const json &packet);
//     static GroupMsgRecall parse_group_recall(const json &packet);
//     static GroupPoke parse_group_poke(const json &packet);
//     static FriendPoke parse_friend_poke(const json &packet);
//     static GroupLuckyKing parse_group_lucky_king(const json &packet);
// };

using NoticeEvent =
    std::variant<FriendMsgRecall, GroupMsgRecall, GroupMemberJoin,
                 GroupMemberLeave, GroupPoke, FriendPoke, GroupLuckyKing>;
} // namespace notice

namespace request {

using RequestFlag = std::string;

// 添加好友请求
struct FriendRequest {
  Qid user_id;            // 请求者 qq 号
  std::u32string comment; // 验证信息
  RequestFlag flag;       // 处理该事件时使用的标志
};

enum class GroupReqSubType {
  Add,    // 加群请求
  Invite, // 被邀请加入群
};

// 入群/被邀请加入群事件
struct GroupRequest {
  GroupReqSubType type;
  Qid group_id, user_id;  // 群号， 加群者 qq / 自身 qq
  std::u32string comment; // 验证信息
  RequestFlag flag;       // 处理该事件时使用的标志
};

// struct RequestEventParser
// {
//     static FriendRequest parse_friend(const json &packet);
//     static GroupRequest parse_group(const json &packet);
// };

using RequestEvent = std::variant<FriendRequest, GroupRequest>;
} // namespace request

namespace meta {
namespace detail {
// struct Statistic
// {
//     size_t pk_recv, pk_send, msg_recv, msg_send;
//     unsigned pk_lost, tcp_disc_times, drop_times;
// };

// struct Status
// {
//     bool good;
//     Statistic statistics;
// };
} // namespace detail

struct HeartBeat {
  int64_t time;     // 时间戳
  int64_t interval; // 到下次心跳的间隔， ms

  /// TODO:
  // 这个字段在不同的 onebot 实现上并不一致， 因此暂未被使用
  // Status status;
};

enum class LifeCycSubType {
  AYAN_UNSUPPORTED_FEATURE Enable,  // HTTP Post (unsupported)
  AYAN_UNSUPPORTED_FEATURE Disable, // HTTP Post (unsupported)
  Connect,                          // Websocket 连接建立
};

struct LifeCycle {
  LifeCycSubType subtype; // 子类型
  int64_t time;           // 时间戳
};

using MetaEvent = std::variant<HeartBeat, LifeCycle>;
} // namespace meta

using namespace message;
using namespace notice;
using namespace request;
using namespace meta;
} // namespace onebot

namespace ayan {

using Event = std::variant<onebot::MetaEvent, onebot::NoticeEvent,
                           onebot::MsgEvent, onebot::RequestEvent>;

template <typename T>
concept IsEvent = requires(T _) {
                    {
                      std::is_same_v<T, onebot::MetaEvent> ||
                          std::is_same_v<T, onebot::NoticeEvent> ||
                          std::is_same_v<T, onebot::RequestEvent> ||
                          std::is_same_v<T, onebot::MsgEvent>
                    };
                  };
} // namespace ayan

//     struct EventParser
//     {
//         static Event parse(const json &packet);
//         static MetaEvent parse_meta(const json &packet);
//         static NoticeEvent parse_notice(const json &packet);
//         static MessageEvent parse_message(const json &packet);
//         static RequestEvent parse_request(const json &packet);
//     };

//     struct EventRoughDumper
//     {
//         std::string operator()(MetaEvent &meta);
//         std::string operator()(NoticeEvent &meta);
//         std::string operator()(MessageEvent &meta);
//         std::string operator()(RequestEvent &meta);
//     };
// }

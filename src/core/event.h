#pragma once

#include "fwd.h"
#include "message.h"

#include <optional>
#include <vector>

/// message event
namespace ayan
{
    template <typename T, typename S>
    struct MessageBase
    {
        T subtype;
        S sender;
        int64_t time;
        MsgId msgid;
        Message message;
    };

    struct PrivateMessageSender
    {
        Qid user_id;
        std::wstring nickname;
        std::wstring sex;
        int age;
    };

    enum class PrivateSubType
    {
        Friend,
        Group,
        GroupSelf,
        Other,
    };

    using PrivateMessage = MessageBase<PrivateSubType, PrivateMessageSender>;

    enum class GroupMsgSubType
    {
        Normal,
        Anonymous,
        Notice,
    };

    enum class GroupRole
    {
        Member,
        Admin,
        Owner,
    };

    struct GroupMessageSender : public PrivateMessageSender
    {
        std::wstring card;
        std::wstring area;
        std::wstring level;
        std::wstring title;
        GroupRole role;
    };

    struct GroupMessage : public MessageBase<GroupMsgSubType, GroupMessageSender>
    {
        Qid group_id;
    };

    struct MessageEventParser
    {
        static PrivateMessage parse_private(const json &packet);
        static GroupMessage parse_group(const json &packet);
        static Message parse_message(const json &msg_field);
    };

    using MessageEvent = std::variant<PrivateMessage, GroupMessage>;
} /// end message event

/// meta event
namespace ayan
{
    struct Statistic
    {
        size_t pk_recv, pk_send, msg_recv, msg_send;
        unsigned pk_lost, tcp_disc_times, drop_times;
    };

    struct Status
    {
        bool good;
        Statistic statistics;
    };

    struct HeartBeat
    {
        int64_t time;
        int64_t interval; /// ms
        Status status;
    };

    enum class LifeCycSubType
    {
        // Enable,   /// HTTP Post (unsupported feature)
        // Disable,  /// HTTP Post (unsupported feature)
        Connect, /// Websocket
    };

    struct LifeCycle
    {
        int64_t time;
        LifeCycSubType subtype;
    };

    struct MetaEventParser
    {
        static HeartBeat parse_heartbeat(const json &packet);
        static Status parse_status(const json &status_field);
        static Statistic parse_statistic(const json &stat_field);
        static LifeCycle parse_lifecycle(const json &packet);
    };

    using MetaEvent = std::variant<HeartBeat, LifeCycle>;

} /// end meta event

/// notice event
namespace ayan
{
    struct FriendMsgRecall
    {
        Qid user_id;
        MsgId msg_id;
    };

    struct GroupMsgRecall
    {
        Qid group_id, operator_id;
        MsgId msg_id;
    };

    enum class JoinType
    {
        Approve,
        Invite,
    };

    struct GroupMemberJoin
    {
        JoinType type;
        Qid group_id, operator_id, user_id;
    };

    enum class LeaveType
    {
        Leave,
        Kick,
        Kick_me,
    };

    struct GroupMemberLeave
    {
        LeaveType type;
        Qid group_id, operator_id, user_id;
    };

    struct GroupPoke
    {
        Qid group_id, user_id, target_id;
    };

    struct FriendPoke
    {
        Qid sender_id;
    };

    struct GroupLuckyKing
    {
        Qid group_id, user_id, king_id;
    };

    struct NoticeEventParser
    {
        static GroupMemberLeave parse_group_leave(const json &packet);
        static GroupMemberJoin parse_group_join(const json &packet);
        static FriendMsgRecall parse_friend_recall(const json &packet);
        static GroupMsgRecall parse_group_recall(const json &packet);
        static GroupPoke parse_group_poke(const json &packet);
        static FriendPoke parse_friend_poke(const json &packet);
        static GroupLuckyKing parse_group_lucky_king(const json &packet);
    };

    using NoticeEvent = std::variant<FriendMsgRecall,
                                     GroupMsgRecall,
                                     GroupMemberJoin,
                                     GroupMemberLeave,
                                     GroupPoke,
                                     FriendPoke,
                                     GroupLuckyKing>;

} /// end notice event

/// request event
namespace ayan
{
    struct FriendRequest
    {
        Qid user_id;
        std::string comment, flag;
    };

    enum class GroupReqSubType
    {
        Add,
        Invite,
    };

    struct GroupRequest
    {
        GroupReqSubType type;
        Qid group_id, user_id;
        std::string comment, flag;
    };

    struct RequestEventParser
    {
        static FriendRequest parse_friend(const json &packet);
        static GroupRequest parse_group(const json &packet);
    };

    using RequestEvent = std::variant<FriendRequest, GroupRequest>;

} /// end request event

namespace ayan
{
    using Event = std::variant<MetaEvent, NoticeEvent, MessageEvent, RequestEvent>;
    using OptEvent = std::optional<Event>;

    template <typename T>
    concept IsEvent = requires(T _)
    {
        {std::is_same<T, MetaEvent>() ||
         std::is_same<T, NoticeEvent>() ||
         std::is_same<T, RequestEvent>() ||
         std::is_same<T, MessageEvent>()};
    };

    struct EventParser
    {
        static Event parse(const json &packet);
        static MetaEvent parse_meta(const json &packet);
        static NoticeEvent parse_notice(const json &packet);
        static MessageEvent parse_message(const json &packet);
        static RequestEvent parse_request(const json &packet);
    };

    struct EventRoughDumper
    {
        std::string operator()(MetaEvent &meta);
        std::string operator()(NoticeEvent &meta);
        std::string operator()(MessageEvent &meta);
        std::string operator()(RequestEvent &meta);
    };
}

#pragma once

#include "ayan/core/bot.h"

namespace ayan {

#define try_invoke(handler, param)                                                       \
  if (handler) {                                                                         \
    std::invoke(handler, bot, param);                                                    \
  }

/// TODO:
/// add `on_install / on_uninstall interface`
/// 

class ServiceAdaptor : public service::ServiceImpl<ServiceAdaptor> {
public:
  using ExecResultType = void;

private:
  template <typename EventStruct>
  using Handler = std::function<void(Bot *, const EventStruct &)>;

public:
  RunResult run(const Shared<Bot> &botptr, Event &event) {
    Bot *bot = botptr.get();

    std::visit(
        Match{
            [bot, this](MsgEvent &msg) {
              std::visit(
                  Match{
                      [bot, this](onebot::PrivateMessage &pmsg) {
                        try_invoke(this->OnMessage.Private, pmsg);
                      },

                      [bot, this](onebot::GroupMessage &gmsg) {
                        try_invoke(this->OnMessage.Group, gmsg);
                      }},
                  msg);
            },

            [bot, this](NoticeEvent &ntc) {
              std::visit(
                  Match{
                      [bot, this](onebot::FriendMsgRecall &fr) {
                        try_invoke(this->OnNotice.FriendRecall, fr);
                      },
                      [bot, this](onebot::GroupMsgRecall &gr) {
                        try_invoke(this->OnNotice.GroupRecall, gr);
                      },
                      [bot, this](onebot::GroupMemberJoin &gj) {
                        try_invoke(this->OnNotice.GroupJoin, gj);
                      },
                      [bot, this](onebot::GroupMemberLeave &gl) {
                        try_invoke(this->OnNotice.GroupLeave, gl);
                      },

                      [](auto &_) {
                        /// TODO:
                        /// add other case
                      },
                  },
                  ntc);
            },

            [bot, this](MetaEvent &meta) {
              std::visit(
                  Match{
                      [bot, this](onebot::HeartBeat &hb) {
                        try_invoke(this->OnMeta.Heartbeat, hb);
                      },
                      [bot, this](onebot::LifeCycle &lc) {
                        try_invoke(this->OnMeta.Lifecycle, lc);
                      },
                  },
                  meta);
            },

            [bot, this](RequestEvent &req) {
              std::visit(
                  Match{
                      [bot, this](onebot::FriendRequest &fr) {
                        try_invoke(this->OnRequest.Friend, fr);
                      },
                      [bot, this](onebot::GroupRequest &gr) {
                        try_invoke(this->OnRequest.Group, gr);
                      },
                  },
                  req);
            },

        },
        event);

    return RunResult::nothing();
  }

public:
  struct OnMessageHandlers {
    Handler<onebot::message::PrivateMessage> Private;
    Handler<onebot::message::GroupMessage>   Group;
  } OnMessage;

  struct OnRequestHandlers {
    Handler<onebot::request::FriendRequest> Friend;
    Handler<onebot::request::GroupRequest>  Group;
  } OnRequest;

  struct OnMetaHandlers {
    Handler<onebot::meta::HeartBeat> Heartbeat;
    Handler<onebot::meta::LifeCycle> Lifecycle;
  } OnMeta;

  struct OnNoticeHandlers {
    Handler<onebot::notice::FriendMsgRecall>  FriendRecall;
    Handler<onebot::notice::GroupMsgRecall>   GroupRecall;
    Handler<onebot::notice::GroupMemberJoin>  GroupJoin;
    Handler<onebot::notice::GroupMemberLeave> GroupLeave;
  } OnNotice;
};

} // namespace ayan

#undef try_invoke
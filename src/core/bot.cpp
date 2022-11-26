#include "ayan/core/bot.h"
#include "ayan/core/envir.h"
#include "ayan/onebot/serde.h"
#include "ayan/service/refresh.h"

namespace ayan {

BotBuilder::BotBuilder(Envir &env) : home_(env.shared_from_this()) {}

BotBuilder BotBuilder::from(Envir &env) {
  return BotBuilder(env);
}

BotPtr BotBuilder::build() {
  auto event_cli = std::make_unique<WebSocketClient>(home_->pool_.nextLoop());
  auto api_cli   = std::make_unique<WebSocketClient>(home_->pool_.nextLoop());

  auto event_addr = fmt::format("{}", netaddr_);
  auto api_addr   = fmt::format("{}api", netaddr_);

  event_cli->open(event_addr.c_str());
  api_cli->open(api_addr.c_str());

  std::string botname = botname_.value_or(
      /// HAHAHA:
      /// just for fun :)
      /// fmt::format("0x{0:x}", reinterpret_cast<size_t>(this))
      std::string(config::kDefaultBotName));

  auto bot = std::make_shared<Bot>(
      id_, std::move(botname), home_, std::move(event_cli), std::move(api_cli), logset_);

  home_->registry(bot);
  return bot;
}

Bot &Bot::run() {
  running_ = true;

  // lazy init on the first call of run()
  std::call_once(init_, [this]() {
    auto ews    = this->event_.get();
    ews->onopen = [this, ews]() {
      this->log("Connected to Onebot Event Server at: {}", ews->url);
    };

    ews->onmessage = [this](const std::string &msg) {
      // this->dbg("on raw event: {}", msg);

      json pkt = json::parse(msg);
      // this->dbg("on json event : {}", pkt.dump(4));

      Event *pev = new Event(EventParser::from_raw(pkt));
      // this->dbg("on event: {:x}, index: {}", reinterpret_cast<size_t>(pev),
      //           pev->index());

      // handle event in worker thread
      this->home_->next_loop()->queueInLoop([this, pev]() {
        this->handle_event(pev);
        delete pev;
      });
    };

    ews->onclose = [this, ews]() {
      this->log("Disconnected to OneBot Event Server at: {}", ews->url);
    };

    auto aws    = this->api_.get();
    aws->onopen = [this, aws]() {
      this->log("Connected to Onebot Api Server at: {}", aws->url);
    };

    aws->onmessage = [this](const std::string &msg) {
      // this->dbg(srcloc, "on raw callret: {}", msg);

      json *pkt = new json(json::parse(msg));
      // this->dbg(srcloc, "on json callrt: {}", pkt->dump(4));

      // handle event in worker thread
      this->home_->next_loop()->queueInLoop([this, pkt]() {
        this->handle_callret(pkt);
        delete pkt;
      });
    };

    aws->onclose = [this, aws]() {
      this->log("Disconnected to OneBot Api Server at: {}", aws->url);
    };

    // use default reconnect setting
    reconn_setting_t recon;
    reconn_setting_init(&recon);

    ews->setReconnect(&recon);
    aws->setReconnect(&recon);

    // registry `RefreshAll` service to clear all exec result cache of services
    this->subscribe<RefreshAll>();

    // this->dbg("bot: {} init success", this->name());
  });

  return log(srcloc, "bot: {} has started!", this->name());
}

Bot &Bot::stop() {
  this->running_ = false;
  return *this;
}

void Bot::handle_event(Event *ev) {
  if (this->is_running()) {
    // this->dbg(srcloc, "handle event,  index: {}", ev->index());
    this->scheduler_->serve(this->shared_from_this(), *ev);
  }
}

void Bot::handle_callret(json *ret) {
  if (this->is_running()) {
    // this->dbg(srcloc, "handle api callret: {}", ret->dump(4));
    if (ret->contains("echo") || ret->contains("retcode")) {
      auto callid = ret->at("echo").get<CallID>();
      CallManager::global().trig_hook(callid, ret->at("data"));
    }
  }
}
} // namespace ayan

/*
  // 匹配所有类型的事件
  auto event_handler = Match{

      // 消息事件
      [this](MsgEvent &msge) {
        auto msg_handler = Match{

            // 处理私聊消息
            [this](onebot::PrivateMessage &pmsg) {
              // 提取消息中所有的文本信息， 如果没有则给定默认值
              std::u32string msgstr =
                  pmsg.message.flatten()
                      .take(1)
                      .take_first<onebot::TextSeg>()
                      .value_or(onebot::TextSeg{.text = "-> _ <- 空消息"_utf8})
                      .text;

              // 输出到控制台
              log(srcloc, "msg id:{}, content:{}", //
                  pmsg.msgid, utf8::utf32to8(msgstr));

              this->onebot_api().send_private_msg(
                  {2821006329, pmsg.message}, [this](onebot::MsgId id) {
                    this->dbg("received call ret of msgid: {}", id);
                  });

              // 回复默认消息， 附带图片
              // this->call_api()
              //   .reply(
              //     pmsg.message.id(),
              //     MessageBuilder::from("这是测试回复"_utf8)
              //       .text("添加一段文字")
              //       .image_local("D:/Github/AyanBot/image/ayan.png")
              //
  .image_url("https://th.bing.com/th/id/R.0981f3b3375ca2c0e303f899c305c7b9?rik=CLBIeaMw8z80zQ&pid=ImgRaw&r=0")
              //       .text("other text")
              //       .build()
              //   );
            },

            // 处理群聊消息， 忽略
            [](onebot::GroupMessage &_) {
              // do nothing
            }};

        std::visit(msg_handler, msge);
      },

      // 其他类型的事件， 忽略
      [this](auto &_) { this->dbg("not private msg event"); }};

  std::visit(event_handler, *ev);
 */
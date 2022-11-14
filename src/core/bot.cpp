#include "ayan/core/bot.h"
#include "ayan/core/envir.h"

#include "client/WebSocketClient.h"

#include <memory>
#include <string>

namespace ayan {

BotBuilder::BotBuilder(Envir &env) : home_(env.shared_from_this()) {}

BotBuilder BotBuilder::from(Envir &env) { return BotBuilder(env); }

BotPtr BotBuilder::build() {
  auto cli = std::make_unique<WebSocketClient>(home_->pool_.nextLoop());

  cli->open(netaddr_.c_str());
  
  std::string botname = 
    botname_.value_or(
      std::string(config::kDefaultBotName)

      // just for fun 
      // fmt::format("0x{0:x}", reinterpret_cast<size_t>(this))
    );

  auto ret = std::make_shared<Bot>(
      id_, std::move(botname), home_,
      std::move(cli), logset_);

  home_->registry(ret);

  return ret;
}

Bot& Bot::run() {
  running_ = true;

  // lazy init on the first call of run()
  std::call_once(init_, [this](){
    auto ws = this->wss_.get();

    ws->onopen = [this, ws]() {
      if (ws->isConnected()) {
      this->log("Connected to OnebotServer at: {}", ws->url);
      }else {
      this->dbg("Disconnected to OneBotServer at: {}", ws->url);
      }
    };

    ws->onmessage = [this] (const std::string& msg) {
      this->log("on message : {}", msg);
    };

    reconn_setting_t recon;
    reconn_setting_init(&recon);
    ws->setReconnect(&recon);

    this->dbg("bot: {} init success", this->name());

  });

  return dbg(srcloc, "bot:{} has started!", this->name());
}

} // namespace ayan
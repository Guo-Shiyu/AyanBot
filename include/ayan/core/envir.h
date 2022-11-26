#pragma once

#include "ayan/core/bot.h"

namespace ayan {

class Envir : public std::enable_shared_from_this<Envir> {
  friend BotBuilder;

public:
  constexpr static size_t kDefaultPoolSize = 2;

public:
  Envir(const Envir &) = delete;

  static Envir &global() {
    static auto G = std::make_shared<Envir>(kDefaultPoolSize);
    return *G;
  }

private:
  Envir() = delete;

public:
  Envir(int threads) : pool_(threads > 0 ? threads : kDefaultPoolSize) {
    // disable libhv's default log
    logger_set_level(hv_default_logger(), LOG_LEVEL_SILENT);

    // wait event loop started
    pool_.start(true);
  }

protected:
  Envir &registry(const BotPtr &bot) {
    bots_[bot->qid()] = bot->shared_from_this();
    return *this;
  }

public:
  size_t threads() {
    return pool_.threadNum();
  }
  hv::EventLoopPtr next_loop() {
    return pool_.nextLoop();
  };

private:
  EventLoopThreadPool        pool_;
  std::map<Qid, Shared<Bot>> bots_;
};

} // namespace ayan

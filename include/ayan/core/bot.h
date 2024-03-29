#pragma once

#include "ayan/import/fmt.h"
#include "ayan/import/optional.h"
#include "ayan/import/websocket.h"

#include "ayan/core/callapi.h"
#include "ayan/core/logger.h"
#include "ayan/core/service.h"

namespace ayan {

using Qid = onebot::Qid;

class Envir;

class Bot : public std::enable_shared_from_this<Bot> {
public:
  friend class BotBuilder;
  using Self = Bot;

private:
  Bot() = delete;

public:
  Bot(const Bot &) = delete;
  Bot(Qid id, std::string &&name, const Shared<Envir> &env,
      Unique<WebSocketClient> &&event, Unique<WebSocketClient> &&api, Logger &&logger)
      : id_(id), selfname_(std::forward<std::string>(name)), home_(env),
        scheduler_(std::make_unique<ServiceSecheduler>()),
        event_(std::forward<decltype(event)>(event)),
        api_(std::forward<decltype(api)>(api)), lgr_(logger) {}

public:
  // 开启和关闭机器人， 关闭时只会让其忽略所有事件和调用相应，
  // 并不会断开与协议适配器的连接， 或者从环境中卸载
  Self &run();
  Self &stop();

  bool is_running() {
    return running_;
  }

  // 获取基本信息
  Qid qid() const {
    return id_;
  }
  std::string_view name() const {
    return selfname_;
  }

public:
  ApiCaller onebot_api() {
    return ApiCaller(this->api_->channel, this->api_->loop());
  }

public:
  template <typename Sev, typename... Args>
  Self &subscribe(Args &&...args) {
    auto sev = std::make_shared<Sev>(args...);
    return subscribe_raw(std::move(sev));
  }

  template <typename Sev>
  Self &subscribe() {
    bool service_registry_call = Sev::template SevRegister<Sev>::Registered;

    auto &support = service::inner::all_available_services();
    auto  sev     = std::invoke(support.at(util::type_name<Sev>()));
    return subscribe_raw(std::move(sev));
  }

  Self &subscribe_raw(ServicePtr &&sev) {
    sev->install(shared_from_this());
    scheduler_->append(sev);
    return *this;
  }

  template <typename... Args>
  Self &log(const char *udata_fmt, Args &&...args) {
    auto userdata = fmt::format(fmt::runtime(udata_fmt), std::forward<Args>(args)...);
    lgr_.log(fmt::format(LogFmt, selfname_, userdata));
    return *this;
  }

  template <typename... Args>
  Self &log(Srcloc loc, const char *udata_fmt, Args &&...args) {
    auto userdata = fmt::format(fmt::runtime(udata_fmt), std::forward<Args>(args)...);
    lgr_.log(loc, fmt::format(LogFmt, selfname_, userdata));
    return *this;
  }

  template <typename... Args>
  Self &err(const char *udata_fmt, Args &&...args) {
    auto userdata = fmt::format(fmt::runtime(udata_fmt), std::forward<Args>(args)...);
    lgr_.err(fmt::format(LogFmt, selfname_, userdata));
    return *this;
  }

  template <typename... Args>
  Self &err(Srcloc loc, const char *udata_fmt, Args &&...args) {
    auto userdata = fmt::format(fmt::runtime(udata_fmt), std::forward<Args>(args)...);
    lgr_.err(loc, fmt::format(LogFmt, selfname_, userdata));
    return *this;
  }

  template <typename... Args>
  Self &dbg(const char *udata_fmt, Args &&...args) {
    auto userdata = fmt::format(fmt::runtime(udata_fmt), std::forward<Args>(args)...);
    lgr_.dbg(fmt::format(LogFmt, selfname_, userdata));
    return *this;
  }

  template <typename... Args>
  Self &dbg(Srcloc loc, const char *udata_fmt, Args &&...args) {
    auto userdata = fmt::format(fmt::runtime(udata_fmt), std::forward<Args>(args)...);
    lgr_.dbg(loc, fmt::format(LogFmt, selfname_, userdata));
    return *this;
  }

private:
  void handle_event(Event *ev);
  void handle_callret(json *ret);

private:
  Qid                       id_;
  const std::string         selfname_;
  Shared<Envir>             home_;
  Unique<ServiceSecheduler> scheduler_;
  Unique<WebSocketClient>   event_, api_;
  Logger                    lgr_;
  std::atomic<bool>         running_ = false;
  std::once_flag            init_    = {};

private:
  constexpr static const char
      // [<bot name>]: <user data>
      *LogFmt = "[{0}]: {1}";
};

using BotPtr = Shared<Bot>;

class BotBuilder {
public:
  using Self = BotBuilder;

private:
  BotBuilder() = delete;

public:
  BotBuilder(const BotBuilder &) = default;
  BotBuilder(Envir &env);

private:
  Optional<std::string> botname_ = NullOpt;          // bot's name
  LogRedirectSet        logset_  = LogRedirectSet(); // log redirect setting
  std::string           netaddr_ = "";               // onebot-adapter server address
  Shared<Envir>         home_;                       // environment born from
  Qid                   id_;

public:
  Self &with_name(std::string_view name) {
    botname_ = name;
    return *this;
  }

  Self &with_qid(Qid id) {
    id_ = id;
    return *this;
  }

  // eg: "127.0.0.1", 6700
  Self &connect(std::string_view ip, unsigned int port) {
    netaddr_ = fmt::format("ws://{}:{}/", ip, port);
    return *this;
  }

  // eg: ws://127.0.0.1:6700/
  Self &connect(std::string_view url) {
    netaddr_ = url;
    return *this;
  }

  Self &set_dbg(const LogStreamPtr &os) {
    logset_.dbg_ = os;
    return *this;
  }

  Self &set_log(const LogStreamPtr &os) {
    logset_.log_ = os;
    return *this;
  }

  Self &set_err(const LogStreamPtr &os) {
    logset_.err_ = os;
    return *this;
  }

  BotPtr build();

  static BotBuilder from(Envir &env);
};
} // namespace ayan

#pragma once

#include "ayan/core/envir.h"

#include <future>
#include <random>

namespace ayan {

using json = nlohmann::json;

class ApiCall {
public:
  ApiCall() = delete;
  ApiCall(const ApiCall &) = default;
  ~ApiCall() = default;

protected:
  explicit ApiCall(const BotPtr &bot);

public:
  static ApiCall from(const BotPtr &bot);
  
  constexpr static int IgnoreResultFlag = 0;

public:
  template <typename T> ApiCall &params(std::string_view key, T &&value) {
    _packet["params"][key.data()] = value;
    return *this;
  }

  ApiCall &action(std::string_view api_name);

  template <typename R>
  std::future<R> get(const std::function<R(const json &)> &unpack_fn) {
    std::promise<R> *pms = new std::promise<R>();
    std::future<R> ret = pms->get_future();

    auto on_ready = [=](const json &packet) {
      try {
        /// specialize
        if constexpr (std::is_same<R, void>()) {
          std::invoke(unpack_fn, packet);
          pms->set_value();
        } else {
          R call_result = std::invoke(unpack_fn, packet);
          pms->set_value(call_result);
        }
      } catch (const std::exception &e) {
        pms->set_exception(std::make_exception_ptr(e));
      }
      delete pms;
    };

    size_t echo_code =
        random() + 1; /// avoid 0, which is ApiCall::IgNoreResultFlag
    _bot->set_hook(echo_code, on_ready);
    call(echo_code);
    return ret;
  }

  template <typename R>
  void handle(const std::function<R(const json &)> &unpack_fn,
              const std::function<void(R &&)> &handle) {
    auto on_ready = [=](const json &packet) {
      /// specialize for 'void'
      if constexpr (std::is_same<R, void>())
        std::invoke(handle);
      else
        std::invoke(handle, std::invoke(unpack_fn, packet));
    };

    size_t echo_code =
        random() + 1; /// avoid 0, which is ApiCall::IgNoreResultFlag
    _bot->set_hook(echo_code, on_ready);
    call(echo_code);
  }

  ApiCall &clear_packet();
  ApiCall &remove_param(std::string_view key);
  void call(size_t echo_code = ApiCall::IgnoreResultFlag);

  ApiWarpper &api();

protected:
  ApiCall &echo(size_t ecode) {
    _packet["echo"] = ecode;
    return *this;
  }

private:
  static std::default_random_engine random;

protected:
  json _packet;
  BotPtr _bot;
};

#ifdef _MSC_VER
#define FORCE_ESO __declspec(empty_bases)
#else
#define FORCE_ESO
#endif // _MSC_VER

template <typename... ApiImpl>
class FORCE_ESO ApiImplMixin : private ApiCall, public ApiImpl... {
public:
  ApiImplMixin() = delete;
  ApiImplMixin(const ApiImplMixin &) = default;
  ~ApiImplMixin() = default;

protected:
  ApiImplMixin(const BotPtr &bot) : ApiCall(bot), ApiImpl()... {}

public:
  static ApiImplMixin from(const BotPtr &bot) { return ApiImplMixin(bot); }
};
} // namespace ayan
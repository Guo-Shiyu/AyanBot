#pragma once

#include "ayan/import/eventloop.h"
#include "ayan/import/websocket.h"

#include "ayan/onebot/message.h"

#include <functional>
#include <future>
#include <random>

namespace onebot {

// onebot api 返回值处理函数
using HookFn = std::function<void(const json &)>;

// 对应每次调用 onebot api 的 echo 字段
using CallID = int;

} // namespace onebot

namespace ayan {
using onebot::CallID;
using onebot::HookFn;

template <typename T>
using Continuation = std::function<void(T &&)>;

constexpr auto Ignore = [](auto &&_) {};

class CallManager {
public:
public:
  CallManager(const CallManager &) = delete;

private:
  CallManager() = default;

public:
  static CallManager &global() {
    static CallManager G{};
    return G;
  }

public:
  CallManager &set_hook(CallID id, HookFn &&hook) {
    handlers_[id] = hook;
    return *this;
  };

  CallManager &trig_hook(CallID id, const json &data) {
    if (handlers_.contains(id)) {
      std::invoke(handlers_[id], data);
      handlers_.erase(id);
    }
    return *this;
  }

private:
  std::map<CallID, HookFn> handlers_;
};

} // namespace ayan

namespace onebot {

using json = nlohmann::json;

class ApiCall {
  friend class Bot;

public:
  ApiCall()                = delete;
  ApiCall(const ApiCall &) = delete;

protected:
  explicit ApiCall(const WebSocketChannelPtr &channel, const EventLoopPtr &loop)
      : channel_(channel), eloop_(loop) {}

public:
  constexpr static int IgnoreResultFlag = 0;

public:
  template <typename T>
  ApiCall &params(std::string_view key, T &&value) {
    packet_["params"][key.data()] = value;
    return *this;
  }

  ApiCall &action(std::string_view api_name) {
    packet_["action"] = api_name;
    return *this;
  }

  void call(size_t echo_code = ApiCall::IgnoreResultFlag) {
    echo(echo_code).channel_->send(packet_.dump());
  }

  template <typename R>
  std::future<R> get(const std::function<R(const json &)> &unpack_fn) {
    std::promise<R> *pms = new std::promise<R>();
    std::future<R>   ret = pms->get_future();

    auto on_ready = [=](const json &packet) {
      try {
        /// specialize for void
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

    size_t echo_code = random() + 1; // avoid 0, which is ApiCall::IgNoreResultFlag
    ayan::CallManager::global().set_hook(echo_code, on_ready);
    call(echo_code);
    return ret;
  }

  template <typename R>
  void handle(
      const std::function<R(const json &)> &unpack_fn,
      const std::function<void(R &&)>      &handle) {
    auto on_ready = [=](const json &packet) {
      /// specialize for 'void'
      if constexpr (std::is_same<R, void>())
        std::invoke(handle);
      else
        std::invoke(handle, std::invoke(unpack_fn, packet));
    };

    size_t echo_code = random() + 1; // avoid 0, which is ApiCall::IgNoreResultFlag
    ayan::CallManager::global().set_hook(echo_code, on_ready);
    call(echo_code);
  }

protected:
  ApiCall &echo(size_t ecode) {
    packet_["echo"] = ecode;
    return *this;
  }

  // private:
  //   static std::default_random_engine random;

protected:
  json                packet_;
  WebSocketChannelPtr channel_;
  EventLoopPtr        eloop_;
};

namespace api {

using ayan::Continuation;

/// forward declare
class ApiCaller;

/// NOTICE:
/// some helper unpack function for unpack callret

template <typename T, const char *Field>
  requires(std::is_fundamental<T>::value)
auto take_field = [](const json &pkt) -> T { return pkt[Field].get<T>(); };

template <typename T>
auto take_full = [](const json &pkt) -> T { return pkt.get<T>(); };

template <typename T, template <typename I> class C>
C<T> take_all(const json &pkt) {
  C<T> ret;
  ret.reserve(pkt.size());
  for (const auto &elem : pkt)
    ret.push_back(take_full<T>(elem));
  return ret;
}

/// NOTICE:
/// all structs in 'api' namespace are required to be empty,
/// which means 'static_assert(sizeof(< STRUCT >)==0);',
/// to enable some magic operation in Mixin-Inheritance

struct SendPrivateMessage {
  using CallRet = MsgId;
  using Params  = std::tuple<Qid, const Message &>;

  std::future<CallRet> send_private_msg(Qid qq, const Message &msg);
  ApiCaller           *send_private_msg(const Params &args, Continuation<CallRet> &&cont);
};

struct SendGroupMessage {
  using CallRet = MsgId;
  using Params  = std::tuple<Qid, const Message &>;

  std::future<MsgId> send_group_msg(Qid qq, const Message &msg);
  ApiCaller         &send_group_msg(const Params &args, Continuation<CallRet> &&cont);
};

} // namespace api
/// NOTICE:
/// 这是 MSVC 的 BUG, EBO (空基类优化) 在 Mixin 继承下不一定会发生，
/// 因此需要强制指明编译器进行该操作
/// link: https://learn.microsoft.com/en-us/cpp/cpp/empty-bases?view=msvc-170

#ifdef _MSC_VER
#define FORCE_EBO __declspec(empty_bases)
#else
#define FORCE_EBO
#endif // _MSC_VER

template <typename... ApiImpl>
class FORCE_EBO ApiMixin : private ApiCall, public ApiImpl... {

public:
  ApiMixin()                 = delete;
  ApiMixin(const ApiMixin &) = delete;

public:
  ApiMixin(const WebSocketChannelPtr &channel, const EventLoopPtr &loop)
      : ApiCall(channel, loop), ApiImpl()... {}
};

} // namespace onebot

#include "ayan/onebot/api.h"
#include "ayan/core/callapi.h"

#define to_call   reinterpret_cast<ApiCall *>(this)
#define as_caller reinterpret_cast<ApiCaller *>(this)

namespace onebot {
namespace k {
constexpr static const char msgid[] = "message_id";
}

namespace api {
std::future<MsgId> SendPrivateMessage::send_private_msg(Qid qq, const Message &msg) {
  return to_call->action("send_private_msg")
      .params("user_id", qq)
      .params("message", msg.dump())
      .get<MsgId>(take_field<MsgId, k::msgid>);
}

ApiCaller *
SendPrivateMessage::send_private_msg(const Params &args, Continuation<CallRet> &&cont) {
  auto &[qq, msg] = args;

  to_call->action("send_private_msg")
      .params("user_id", qq)
      .params("message", msg.dump())
      .handle<MsgId>(take_field<MsgId, k::msgid>, cont);

  return as_caller;
}

} // namespace api

} // namespace onebot

#undef to_call
#undef as_caller
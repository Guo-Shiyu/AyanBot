#include "ayan/core/bot.h"

namespace ayan {

RunResult ServiceSecheduler::serve(const Shared<Bot> &bot, Event &event) noexcept {

#ifdef AYAN_TRACE_SERVICE
  bot->dbg(srcloc, "[BEGIN] service `{}`", this->identity());
  prelude().foreach ([&](auto &sep) {
    bot->dbg("[BEGIN] service `{}`", sep.first);
    auto &exec_state  = sep.second;
    exec_state.result = exec_state.sev->serve(bot, event);
    bot->dbg("[END] service `{}`", sep.first);
  });
  bot->dbg(srcloc, "[END] service `{}`", this->identity());
#else
  prelude().foreach ([&](auto &sep) {
    auto &exec_state  = sep.second;
    exec_state.result = exec_state.sev->serve(bot, event);
  });
#endif

  return run(bot, event);
}

} // namespace ayan
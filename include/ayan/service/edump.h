#pragma once

#include "ayan/core/bot.h"

namespace ayan {

class DumpOnebotEvent : public service::ServiceImpl<DumpOnebotEvent> {
public:
  constexpr static service::RetCode ReturnCode = 11514;

  using ExecResultType = std::string;

public:
  RunResult run(const Shared<Bot> &botptr, Event &event) {
    Bot *bot = botptr.get();

    std::string event_type;
    event_type.reserve(64);

    auto get_event_type =
        Match{[&](auto ty) { event_type = util::type_name<decltype(ty)>(); }};

    std::visit(
        Match{
              [&](auto& e) { std::visit(get_event_type, e); }
        },
        event);

    return RunResult{
        .ret   = ReturnCode,
        .extra = std::move(event_type),
    };
  }
};
} // namespace ayan
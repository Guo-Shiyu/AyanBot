#pragma once

#include "ayan/core/bot.h"

namespace ayan {

class RefreshAll : public ServiceImpl<RefreshAll> {
public:
  constexpr static service::RetCode ReturnCode = 221125;

  using ExecResultType = size_t;

public:
  void usuage(ServiceManager *super, ServiceManager *self) {
    super_ = super;
  }

  RunResult run(Shared<Bot> botptr, Event &event) {
    return RunResult{
        .ret   = ReturnCode,
        .extra = super_->invalid_all(),
    };
  }

private:
  ServiceManager *super_;
};
} // namespace ayan

#pragma once

#include "ayan/core/service.h"

namespace ayan {

class RefreshAll : public ServiceImpl<RefreshAll> {
public:
  using ExecResultType = void;

protected:
  void usuage(ServiceManager *super, ServiceManager *self);
  void load(const Shared<Bot> &botptr);
  void unload(const Shared<Bot> &botptr);
  RunResult run(Shared<Bot> botptr, Event &event);
};
} // namespace ayan
#pragma once

#include "ayan/onebot/event.h"

namespace ayan {
struct EventParser {
  static Event from_raw(const json &packet);
};
} // namespace ayan
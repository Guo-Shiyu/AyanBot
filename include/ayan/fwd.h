#pragma once

#include "ayan/config.h"

#define AYAN_UNSUPPORTED_FEATURE

#include <memory>

namespace ayan {
template <typename T> using Shared = std::shared_ptr<T>;
template <typename T> using Unique = std::unique_ptr<T>;
} // namespace ayan
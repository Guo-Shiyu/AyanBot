#pragma once

#include "source_location/source_location.hpp"

#define srcloc nostd::source_location::current()

using Srcloc = nostd::source_location;
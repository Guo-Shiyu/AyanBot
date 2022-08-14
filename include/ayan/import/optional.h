#pragma once

#include "tl/optional.hpp"

constexpr auto NullOpt = tl::nullopt;

template <typename T>
using Optional = tl::optional<T>;

template <typename T>
Optional<T> make_optional(T &&obj)
{
    return tl::make_optional(T);
}

#pragma once

#include "result.hpp"

template <typename T, typename E> using Result = cpp::result<T, E>;

template <typename E> inline auto Err(E &&e) { return cpp::fail(e); }

template <typename T> inline auto Ok(T &&ok) { return ok; }

template <typename T = void> inline auto Ok() {}

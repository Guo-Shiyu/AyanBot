#pragma once

#include <variant>

template <class... Types>
struct Match : Types... {
  using Types::operator()...;
};

template <class... Types>
Match(Types...) -> Match<Types...>;

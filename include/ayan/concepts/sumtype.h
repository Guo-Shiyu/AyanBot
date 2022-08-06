#pragma once

#include <variant>

template <class... Types>
struct Overloaded : Types...
{
    using Types::operator()...;
};

template <class... Types>
Overloaded(Types...) -> Overloaded<Types...>;


#pragma once

#include <iterator>
#include <type_traits>

template <class BeginIt, class EndIt>
concept IterableRange = requires(BeginIt begin_it, EndIt end_it) {
                          { ++begin_it };
                          { *begin_it };
                          { begin_it != end_it };
                          requires !std::is_void_v<decltype(*begin_it)>;
                        };

template <class T>
concept Iterable =
    std::is_array_v<T> ||
    requires(T value) {
      { value.begin() };
      { value.end() };
      requires IterableRange<decltype(value.begin()), decltype(value.end())>;
    } || requires(T value) {
           { std::begin(value) };
           { std::end(value) };
           requires IterableRange<decltype(std::begin(value)), decltype(std::end(value))>;
         };

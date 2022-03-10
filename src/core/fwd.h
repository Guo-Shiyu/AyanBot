#pragma once

#include <cstdint>
#include <memory>

namespace ayan
{
    /// component
    struct RedirectInfo;
    using Qid = uint64_t;
    using MsgId = int32_t;

    /// bot
    class Bot;
    class BotBuilder;
    using BotPtr = std::shared_ptr<Bot>;
    
    /// env
    class Env;
    class EnvBuilder;
    using EnvPtr = std::shared_ptr<Env>;

    /// service    
    struct ServiceConcept;
    template <typename T>
    struct ServiceBase;

    /// message
    class Message;

    /// onebot api
    class ApiCall;
    template<typename... ApiImpl>
    class ApiImplMixin;

    /// other useful concepts and templates
    template <class... Types>
    struct Overloaded : Types...
    {
        using Types::operator()...;
    };

    template <class... Types>
    Overloaded(Types...) -> Overloaded<Types...>;

    template <class BeginIt, class EndIt>
    concept IterableRange = requires(BeginIt begin_it, EndIt end_it) {
        { ++begin_it };
        { *begin_it };
        { begin_it != end_it };
            requires !std::is_void_v<decltype(*begin_it)>;
    };

    template <class T>
    concept Iterable = std::is_array_v<T>
        || requires(T value) {
            { value.begin() };
            { value.end() };
                requires IterableRange<decltype(value.begin()), decltype(value.end())>;
    } || requires(T value) {
        { std::begin(value) };
        { std::end(value) };
            requires IterableRange<decltype(std::begin(value)), decltype(std::end(value))>;
    };

} // namespace ayan

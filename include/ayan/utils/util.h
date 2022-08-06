#pragma once

#include <thread>
#include <chrono>

namespace ayan
{

    using never = void;

    namespace util
    {
        using namespace std::chrono_literals;
        template <typename Time = decltype(1ms)>
        never block_here(Time&& wake_interval = 100ms)
        {
            while (true)
            {
                std::this_thread::sleep_for(wake_interval);
            }
        }

        // /// e.g:
        // /// auto ms = count_ms(1min); 
        // /// assert(ms == 60 * 1000);
        // template <class _Rep, class _Period>
        // auto count_in_ms(const std::chrono::duration<_Rep, _Period> &real_time)
        // {
        //     return std::chrono::duration_cast<decltype(1ms)>(real_time).count();
        // }

        // template <typename... T>
        // int accumulate_in_ms(T &&...real_time)
        // {
        //     return ((count_ms(real_time)) + ...);
        // }

        void clear_screen() noexcept
        {
#ifdef _WIN32
            std::system("chcp 65001 & cls");
#else
            std::system("clear");
#endif
        }
    }
}
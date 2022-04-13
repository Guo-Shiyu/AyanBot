#pragma once

#include "../core/service.h"
#include "../core/apicall.h"

#include "hv/htime.h"

#include <chrono>

using namespace std::chrono_literals;

class AlarmManager : public MessageService<AlarmManager>
{
public:
    using AnsweredState = bool;

    constexpr static std::initializer_list<Qid> UserList =
        {
            2821006329, 
        };

public:
    std::string_view name() const override
    {
        return "AlarmClock Service";
    }

    void on_load(Bot &bot_)
    {
        datetime_t now_datetime = datetime_now();
        time_t now = datetime_mktime(&now_datetime);

        now_datetime.day = now_datetime.day + 1;
        now_datetime.hour = 23;
        now_datetime.min = 59;
        time_t target = datetime_mktime(&now_datetime);
        {
            std::shared_ptr<Bot> bot = bot_.shared_from_this();
            bot->set_timeout((target - now) * 1000, [=](hv::TimerID _)
                             {
                auto msg = MsgBuilder::from(std::string{time_now()})
                               .text("!!!")
                               .build();

                for (auto id : UserList)
                {
                    bot->api().send_private_msg({id, msg});
                    _answered[id] = false;

                    TimerTrace trace = bot->set_interval(30 * 1000, [=](hv::TimerID timer)
                                                         {
                                                            if (not _answered[id])
                                                                 bot->api().send_private_msg({ id, msg }); 
                                                         });

                    bot->set_interval(55 * 1000, [=](hv::TimerID timer)
                                      {
                                        if (_answered[id])
                                            bot->kill_timer(trace); 
                                      });
                } });
        }
    }

    bool trig(Bot &bot, PrivateMessage &msg)
    {
        return _answered.contains(msg.sender.user_id);
    }

    void act(Bot &bot, PrivateMessage &msg)
    {
        _answered[msg.sender.user_id] = true;
    }

    using MessageService<AlarmManager>::trig;
    using MessageService<AlarmManager>::act;

private:
    std::map<Qid, AnsweredState> _answered;
};
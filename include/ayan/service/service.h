#pragma once

#include "ayan/core/event.h"
#include "ayan/utils/util.h"

#include <map>
#include <functional>
#include <any>

namespace ayan
{
    class Bot;

    namespace service
    {
        using RetCode = int;
        using ExtraInfo = std::any;

        struct Success
        {
            constexpr static RetCode _ = std::numeric_limits<RetCode>::max();

            RetCode code = _;
            ExtraInfo extra = {};
        };

        struct Failure
        {
            constexpr static RetCode _ = std::numeric_limits<RetCode>::min();

            RetCode code = _;
            ExtraInfo extra = {};
        };

        struct Exception
        {
            std::exception_ptr eptr;
        };

        using LoadResult = Optional<std::variant<Failure, Exception>>;
        using RunResult = std::variant<Success, Failure, Exception>;

        // template <typename Sev>
        // struct ServiceInfo
        // {
        //     using SuccessCase = typename Sev::SuccessCase;
        //     using FailureCase = typename Sev::FailureCase;
        // };

        struct ServiceConcept
        {
            virtual LoadResult install(Bot &bot) noexcept = 0;            // -> load()
            virtual LoadResult uninstall(Bot &bot) noexcept = 0;          // -> unload()
            virtual RunResult serve(Bot &bot, Event &event) noexcept = 0; // -> run()
        };

        using PreludeSevList = std::vector<std::string>;

        template <typename Sev, typename B = Bot, typename E = Event>
        concept IsServiceImpl = requires(Sev s, B b, E e)
        {
            {s.usuage()};
            {s.load(b)};
            {s.unload(b)};
            {s.run(b, e)};
        };

        namespace inner
        {
            using SevCreator = std::function<ServiceConcept *(void)>;

            template <typename Sev>
            struct ServiceCreator
            {
                ServiceConcept *create()
                {
                    return new Sev();
                }
            };

            using SevSupportMap = std::unordered_map<std::string, SevCreator>;

            static SevSupportMap &get_available_services()
            {
                static SevSupportMap map;
                return map;
            }

            template <typename Sev>
            bool make_available()
            {
                auto &map = get_available_services();
                auto sev_name = util::type_name<Sev>();
                map.emplace(sev_name, &ServiceCreator<Sev>::create);
                return true;
            }

            template <typename SevImpl>
            struct SevRegister
            {
                // friend SevImpl;

                static bool registery()
                {
                    return make_available<SevImpl>();
                }

                static bool Registered;
            };

            template <typename SevImpl>
            bool SevRegister<SevImpl>::Registered = SevRegister<SevImpl>::registery();
        }

        class SevExecuteStack
        {

        public:
            // template <typename... SevImpls>
            // PreludeSevList need()
            // {
            //     PreludeSevList list{};
            //     list.reserve(sizeof...(SevImpls));
            //     (list.emplace_back(util::type_name<SevImpls>()), ...);
            //     return list;
            // }

        private:
            std::map<std::string, std::tuple<ServiceConcept *, Optional<RunResult>>> exec_;
        };

        template <IsServiceImpl Impl>
        struct ServiceBase : public ServiceConcept, public inner::SevRegister<Impl>, protected SevExecuteStack
        {
            LoadResult install(Bot &bot) noexcept override final
            {
                auto *impl = static_cast<Impl *>(this);
                auto preludes = impl->prelude();
                for (auto &sev_name : preludes)
                {
                    std::string name;
                    std::swap(sev_name, name);
                    auto sev_ptr = std::invoke(inner::get_available_services().at[name]);
                    auto &[_, state] = exec_.emplace(std::move(name), std::make_tuple(sev_ptr, NullOpt));

                    {
                        auto &[sevptr, _] = state;
                        sevptr->install(bot);
                    }
                }
                return impl->load(bot);
            }

            LoadResult uninstall(Bot &bot) noexcept override final
            {
                /// TODO:
                return static_cast<Impl *>(this)->unload(bot);
            }

            RunResult serve(Bot &bot, Event &event) noexcept override final
            {
                return static_cast<Impl *>(this)->run(bot, event);
            }

            PreludeSevList prelude()
            {
                // return need();
                return {}
            }
        };
    }

    using service::LoadResult;
    using service::PreludeSevList;
    using service::RunResult;
    using service::ServiceBase;

    class ServiceManager : public ServiceBase<ServiceManager>
    {
    public:
        LoadResult load(Bot &bot);

        LoadResult unload(Bot &bot);
        RunResult run(Bot &bot, Event &event);

        PreludeSevList prelude();
    };
}
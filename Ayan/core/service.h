#pragma once

#include "fwd.h"
#include "env.h"
#include "event.h"
#include "bot.h"

#include <string_view>

namespace ayan
{
	struct ServiceConcept
	{
		virtual ~ServiceConcept() = default;

		virtual std::string_view name() const = 0;
		virtual void install(Bot &bot) = 0;
		virtual void uninstall(Bot &bot) = 0;
		virtual void serve(Bot &bot, OptEvent &event) = 0;

		virtual bool subscribed_by(Qid id) = 0;
		virtual bool available_for(Qid id) = 0;
		virtual bool enable(Bot &bot) = 0;
		virtual bool disable(Bot &bot) = 0;
	};

	template <typename I>
	struct ServiceBase : public ServiceConcept
	{
	public:
		virtual ~ServiceBase() = default;

	public:
		constexpr static bool
			DefaultSwitchState = true,
			Terminable = false,
			Consumptive = false;

	public:
		void install(Bot &bot) override
		{
			auto impl = reinterpret_cast<I *>(this);
			_available.emplace(bot.id(), impl->DefaultSwitchState);
			impl->on_load(bot);
			if (impl->DefaultSwitchState)
				impl->enable(bot);
		}

		void uninstall(Bot &bot) override
		{
			auto impl = reinterpret_cast<I *>(this);
			impl->disable(bot);
			_available.erase(bot.id());
			impl->on_unload(bot);
		}

		void serve(Bot &bot, OptEvent &event) override
		{
			auto impl = reinterpret_cast<I *>(this);
			impl->attempt(bot, event);
		}

		bool subscribed_by(Qid id) override
		{
			return _available.contains(id);
		}

		bool available_for(Qid id) override
		{
			if (_available.contains(id))
				return _available[id];
			else
				return false;
		}

		bool enable(Bot &bot) override
		{
			auto id = bot.id();
			auto impl = reinterpret_cast<I *>(this);
			if (impl->Terminable && _available.contains(id))
			{
				if (_available[id] == false)
					_available[id] = true;
				reinterpret_cast<I *>(this)->on_start(bot);
				return true;
			}
			else
				return false;
		}

		bool disable(Bot &bot) override
		{
			auto id = bot.id();
			auto impl = reinterpret_cast<I *>(this);
			if (impl->Terminable && _available.contains(id) && _available[id])
			{
				_available[id] = false;
				impl->on_stop(bot);
				return true;
			}
			else
				return false;
		}

	public:
		void on_load(Bot &bot)
		{
			bot.log("- {0} has been loaded for {1}", name(), bot.name());
		}

		void on_unload(Bot &bot)
		{
			bot.log("- {0} has been unloaded for {1}", name(), bot.name());
		}

		void on_start(Bot &bot)
		{
			bot.log("- {0} of {1} has started ", name(), bot.name());
		}

		void on_stop(Bot &bot)
		{
			bot.log("- {0} of {1} has stopped ", name(), bot.name());
		}

	protected:
		std::map<Qid, bool> _available = {};
	};

	template <typename Impl, IsEvent Target>
	class UniverseService : public ServiceBase<Impl>
	{
	public:
		using TargetEvent = Target;

	public:
		virtual ~UniverseService() = default;

	public:
		void attempt(Bot &bot, OptEvent &event)
		{
			if (auto &e = event.value(); std::holds_alternative<Target>(e))
			{
				auto impl = static_cast<Impl *>(this);
				auto &target = std::get<Target>(e);
				if (this->_available[bot.id()])
				{
					if (impl->wake(bot, target))
						impl->enable(bot);
				}
				else
				{
					if (impl->sleep(bot, target))
						impl->disable(bot);
				}
				if (impl->handle(bot, target))
					if (impl->Consumptive)
						event.reset();
			}
		}

		bool wake(const Bot &bot, Target &tgeve)
		{
			return false;
		}

		bool sleep(const Bot &bot, Target &tgeve)
		{
			return false;
		}

		bool handle(Bot &bot, Target &event)
		{
			auto impl = static_cast<Impl *>(this);
			if (impl->trig(bot, event))
			{
				impl->act(bot, event);
				return true;
			}
			else
				return false;
		}

		bool trig(const Bot &bot, Target &event)
		{
			return false;
		}

		void act(Bot &bot, Target &event)
		{
		}
	};

	template <typename Impl>
	using MetaService = UniverseService<Impl, MetaEvent>;

	template <typename Impl>
	using NoticeService = UniverseService<Impl, NoticeEvent>;

	template <typename Impl>
	using RequestService = UniverseService<Impl, RequestEvent>;

	template <typename Impl>
	class UniverseService<Impl, MessageEvent> : public ServiceBase<Impl>
	{
	public:
		using TargetEvent = MessageEvent;

	public:
		virtual ~UniverseService() = default;

	public:
		void attempt(Bot &bot, OptEvent &event)
		{
			if (auto &e = event.value(); std::holds_alternative<MessageEvent>(e))
			{
				auto impl = static_cast<Impl *>(this);
				auto &msg_event = std::get<MessageEvent>(e);
				Qid sender = std::visit(Overloaded{[](const PrivateMessage &msg) -> Qid
												   {
													   return msg.sender.user_id;
												   },
												   [](const GroupMessage &msg) -> Qid
												   {
													   return msg.sender.user_id;
												   }},
										msg_event);
				if (impl->Terminable and bot.admins().contains(sender))
					if (this->_available[bot.id()])
					{
						if (impl->wake(bot, msg_event))
							impl->enable(bot);
					}
					else
					{
						if (impl->sleep(bot, msg_event))
							impl->disable(bot);
					}
				if (impl->handle(bot, msg_event))
					if (impl->Consumptive)
						event.reset();
			}
		}

		bool handle(Bot &bot, MessageEvent &event)
		{
			auto impl = static_cast<Impl *>(this);
			auto process = [impl, &bot](auto &msg) -> bool
			{
				if (not impl->filter(bot, msg))
					if (impl->trig(bot, msg))
					{
						impl->act(bot, msg);
						return true;
					}

				return false;
			};

			return std::visit(Overloaded{[=](PrivateMessage &pri_msg) -> bool
										 {
											 return process(pri_msg);
										 },
										 [=](GroupMessage &gro_msg) -> bool
										 {
											 return process(gro_msg);
										 }},
							  event);
		}

		bool wake(const Bot &bot, MessageEvent &tgeve)
		{
			return false;
		}

		bool sleep(const Bot &bot, MessageEvent &tgeve)
		{
			return false;
		}

		bool filter(Bot &bot, PrivateMessage &msg)
		{
			return false;
		}

		bool filter(Bot &bot, GroupMessage &msg)
		{
			return false;
		}

		bool trig(Bot &bot, MessageEvent &event)
		{
			return std::visit(Overloaded{[&, this](PrivateMessage &pri)
										 {
											 return this->trig(pri);
										 },
										 [&, this](GroupMessage &gro)
										 {
											 return this->trig(gro);
										 }});
		}

		bool trig(Bot &bot, PrivateMessage &msg)
		{
			return false;
		}

		bool trig(Bot &bot, GroupMessage &msg)
		{
			return false;
		}

		void act(Bot &bot, PrivateMessage &msg)
		{
		}

		void act(Bot &bot, GroupMessage &msg)
		{
		}
	};

	template <typename Impl>
	using MessageService = UniverseService<Impl, MessageEvent>;

	template <typename... Args>
	constexpr bool all(Args... args)
	{
		return (... && args);
	}

	template <typename... Args>
	constexpr bool any(Args... args)
	{
		return (... || args);
	}

	template <typename... Mixins>
	class MixinService : public Mixins...
	{
	public:
		MixinService() : Mixins()... {}
		virtual ~MixinService() = default;

	public:
		constexpr static bool
			DefaultSwitchState = true,
			Terminable = all(Mixins::Terminable...),
			Consumptive = any(Mixins::Consumptive...);

	public:
		void attempt(Bot &bot, OptEvent &event)
		{
			((Mixins::attempt(bot, event)), ...);
		}

		bool disable(Bot &bot)
		{
			return (... || (Mixins::disable(bot)));
		}

		bool enable(Bot &bot)
		{
			return (... || (Mixins::enable(bot)));
		}

		void on_load(Bot &bot)
		{
			((Mixins::on_load(bot)), ...);
		}

		void on_unload(Bot &bot)
		{
			((Mixins::on_unload(bot)), ...);
		}

		void on_start(Bot &bot)
		{
			((Mixins::on_start(bot)), ...);
		}

		void on_stop(Bot &bot)
		{
			((Mixins::on_stop(bot)), ...);
		}

		template <IsEvent E>
		bool handle(Bot &bot, E &event)
		{
			return _handle_impl<E, Mixins...>(bot, event);
		}

		template <IsEvent E>
		bool wake(Bot &bot, E &event)
		{
			return _wake_impl<E, Mixins...>(bot, event);
		}

		template <IsEvent E>
		bool sleep(Bot &bot, E &event)
		{
			return _sleep_impl<E, Mixins...>(bot, event);
		}

		template <IsEvent E>
		bool trig(Bot &bot, E &event)
		{
			return _trig_impl<E, Mixins...>(bot, event);
		}

		template <IsEvent E>
		bool act(Bot &bot, E &event)
		{
			return _act_impl<E, Mixins...>(bot, event);
		}

	private:
		template <typename E, typename MixFirst, typename... Mix>
		bool _handle_impl(Bot &bot, E &event)
		{
			if constexpr (sizeof...(Mix) > 0)
				if constexpr (std::is_same<MixFirst::TargetEvent, E>())
					return MixFirst::handle(bot, event) && _handle_impl<E, Mix...>(bot, event);

			return false;
		}

		template <typename E, typename MixFirst, typename... Mix>
		bool _wake_impl(Bot &bot, E &event)
		{
			if constexpr (sizeof...(Mix) > 0)
				if constexpr (std::is_same<MixFirst::TargetEvent, E>())
					return MixFirst::wake(bot, event) && _wake_impl<E, Mix...>(bot, event);

			return false;
		}

		template <typename E, typename MixFirst, typename... Mix>
		bool _sleep_impl(Bot &bot, E &event)
		{
			if constexpr (sizeof...(Mix) > 0)
				if constexpr (std::is_same<MixFirst::TargetEvent, E>())
					return MixFirst::sleep(bot, event) && _sleep_impl<E, Mix...>(bot, event);

			return false;
		}

		template <typename E, typename MixFirst, typename... Mix>
		bool _trig_impl(Bot &bot, E &event)
		{
			if constexpr (sizeof...(Mix) > 0)
				if constexpr (std::is_same<MixFirst::TargetEvent, E>())
					return MixFirst::trig(bot, event) && _trig_impl<E, Mix...>(bot, event);

			return false;
		}

		template <typename E, typename MixFirst, typename... Mix>
		bool _act_impl(Bot &bot, E &event)
		{
			if constexpr (sizeof...(Mix) > 0)
				if constexpr (std::is_same<MixFirst::TargetEvent, E>())
					return MixFirst::act(bot, event) && _act_impl<E, Mix...>(bot, event);

			return false;
		}
	};

} // namespace ayan

namespace ayan
{
	namespace default_service
	{
		class KeepAlive : public MetaService<KeepAlive>
		{
		public:
			constexpr static bool
				Consumptive = true,
				DefaultSwitchState = true,
				Terminable = false;

			constexpr static std::string_view SelfName = "KeepAlive";

		public:
			std::string_view name() const override;

			bool handle(Bot &bot, MetaEvent &meta);
		};
	}

}
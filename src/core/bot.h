#pragma once

#include "redirect.h"
#include "event.h"
#include "env.h"
#include "apiimpl.h"

#include "hv/WebSocketClient.h"

#include <set>

namespace ayan
{
	struct _
	{
	};
#define UNCOMMON_INCLUDE
#define AYAN_API(api) api,
	using ApiWarpper = ApiImplMixin<
#include "apiimpl.h"
		_>;
}

namespace ayan
{
	class BotBuilder
	{
	public:
		friend class Bot;

	public:
		BotBuilder &connect(std::string_view &&ip, std::string_view &&port);
		BotBuilder &connect(std::string_view &&url);
		BotBuilder &with_name(const std::string &qid);
		BotBuilder &redirect_log(std::ostream *log);
		BotBuilder &redirect_err(std::ostream *err);
		BotBuilder &redirect_dbg(std::ostream *dbg);
		BotBuilder &redirect_to(const RedirectInfo &info);
		std::shared_ptr<Bot> init();

	private:
		std::shared_ptr<Env> _home;
		std::string _netaddr = {};
		std::string _self = "0x4179616e";
		RedirectInfo _redirect = RedirectInfo::Default();
	};

	using TimerTrace = std::tuple<hv::EventLoopPtr, hv::TimerID>;

	class Bot : public std::enable_shared_from_this<Bot>
	{
	public:
		friend class ApiCall;
		using ServiceAlias = std::string_view;

	protected:
		static constexpr size_t MaxHookTime = 10'000; /// 5s

	private:
		Bot() = delete;
		Bot(const Bot &) = delete;

	public:
		explicit Bot(const std::shared_ptr<Env> &from, std::string &&id, const std::set<Qid> &admins, RedirectInfo redirect, std::string &&netaddr);
		~Bot();

	public:
		static BotBuilder from(const EnvPtr &env);

	public:
		Qid id();

	public:
		Bot &start();
		Bot &stop();
		Bot &suspend();
		Bot &resume();

	public:
		Bot &flush_all();
		ApiCall active();
		ApiWarpper api();

	public:
		Bot &permit(ServiceAlias service);
		Bot &forbid(ServiceAlias service);
		Bot &subscribe(ServiceAlias service);
		Bot &unsubscribe(ServiceAlias service);

	public:
		TimerTrace set_timeout(int timeout_ms, hv::TimerCallback callback);
		TimerTrace set_interval(int interval_ms, hv::TimerCallback callback);
		void kill_timer(const TimerTrace &trace);
		void reset_timer(const TimerTrace &trace);

	protected:
		void on_message(const std::string &full_packet);
		Bot &response(Event &&event);
		Bot &set_hook(int ecode, std::function<void(const json &)> &&hook, unsigned timeout_ms = MaxHookTime);

	private:
		static constexpr const char log_fmt[] = "[{0}] [{1}] [{2}]: {3}\n";
		static constexpr const char
			info[] = "INFO",
			debug[] = "DEBUG",
			error[] = "ERROR";
		std::string _log_impl(const char *level, std::string_view content);

	public:
		template <typename... Args>
		Bot &log(std::string_view fmt, Args &&...args)
		{
			*_redinfo.log << _log_impl(info, std::format(fmt, args...));
			return *this;
		}

		template <typename... Args>
		Bot &dbg(std::string_view fmt, Args &&...args)
		{
			*_redinfo.dbg << "\033[1;32m" << _log_impl(debug, std::format(fmt, args...)) << "\033[0m";
			return *this;
		}

		template <typename... Args>
		Bot &err(std::string_view fmt, Args &&...args)
		{
			*_redinfo.err << "\033[1;31m" << _log_impl(error, std::format(fmt, args...)) << "\033[0m";
			return *this;
		}

	public:
		Env &home();
		std::shared_ptr<hv::WebSocketChannel> &channel();
		RedirectInfo redirect_info() const;
		const std::set<Qid> &admins() const;
		std::string_view name();

	private:
		const std::string _self;
		std::shared_ptr<Env> _home;
		std::set<Qid> _admins;
		RedirectInfo _redinfo;
		std::map<std::string_view, ServiceConcept *> _services;
		std::map<size_t, std::function<void(const json &)>> _hooks;
		const std::string _netaddr;
		hv::WebSocketClient _conn;

	private:
		static constexpr int
			PAUSE = 0,
			WORK = 1;
		std::atomic_int _state;
	};

}

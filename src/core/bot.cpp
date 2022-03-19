#include "bot.h"
#include "service.h"
#include "apicall.h"
#include "utility.h"

namespace ayan
{
	BotBuilder &BotBuilder::connect(std::string_view &&ip, std::string_view &&port)
	{
		/// "ws://127.0.0.1:6700/"
		_netaddr = fmt::format("ws://{0}:{1}/", ip, port);
		return *this;
	}

	BotBuilder &BotBuilder::connect(std::string_view &&url)
	{
		_netaddr = url;
		return *this;
	}

	BotBuilder &BotBuilder::with_name(const std::string &name)
	{
		_self = name;
		return *this;
	}

	BotBuilder &BotBuilder::redirect_log(std::ostream *log)
	{
		_redirect.log = log;
		return *this;
	}

	BotBuilder &BotBuilder::redirect_err(std::ostream *err)
	{
		_redirect.err = err;
		return *this;
	}

	BotBuilder &BotBuilder::redirect_dbg(std::ostream *dbg)
	{
		_redirect.dbg = dbg;
		return *this;
	}

	BotBuilder &BotBuilder::redirect_to(const RedirectInfo &info)
	{
		_redirect = info;
		return *this;
	}

	std::shared_ptr<Bot> BotBuilder::init()
	{
		return std::make_shared<Bot>(_home, std::move(_self), _home->superusers(), _redirect, std::move(_netaddr));
	}

	Bot::Bot(const std::shared_ptr<Env> &from, std::string &&name, const std::set<Qid> &admins, RedirectInfo redirect, std::string &&netaddr)
		: _self(std::forward<std::string>(name)), _home(from), _admins(admins), _redinfo(redirect), _services(), _hooks(), _netaddr(std::forward<std::string>(netaddr)), _conn(), _state(PAUSE)
	{
		auto reconn = hv::ReconnectInfo{};
		reconn.max_delay = 10'000; // 10s
		reconn.min_delay = 1'000;  // 1s
		reconn.delay_policy = 2;

		_conn.setReconnect(&reconn);
	}

	Bot::~Bot()
	{
		if (_state != PAUSE)
			stop();
	}

	BotBuilder Bot::from(const EnvPtr &env)
	{
		BotBuilder tobuild;
		tobuild._redirect = env->redirect_info();
		tobuild._home = env->shared_from_this();
		return tobuild;
	}

	Bot &Bot::start()
	{
		log("initializing from environment: {}, attempt to start...", _home->name());

		_conn.onmessage = [this](const std::string &packet)
		{
			// busy-wait for required service to load
			while (_state != WORK)
				std::this_thread::yield();
			try
			{
				this->on_message(packet);
			}
			catch (const std::exception &e)
			{
				std::string err_info = e.what();
				this->err("on message: {}", err_info);
			}
		};
		_conn.open(_netaddr.c_str());

		std::string_view netaddr = _netaddr;
		netaddr.remove_prefix(5); // "ws://"
		netaddr.remove_suffix(1); // "/"
		log("connected to server: {}", netaddr);

		// busy-wait for websocket channel to connect
		while (not _conn.channel->isConnected())
			std::this_thread::yield();

		log("installing service...");
		_home->registry(shared_from_this());
		for (auto &required : _home->required())
			subscribe(required);

		// start to process event
		_state = WORK;

		return log("bot has started successfully.", _self);
	}

	Bot &Bot::stop()
	{
		log("dropping from environment: {}, attempt to stop...", _home->name());

		while (not _services.empty())
			unsubscribe(_services.begin()->first);

		_state = PAUSE;
		suspend();

		return log("bot has stoppd successfully.").flush_all();
	}

	Bot &Bot::suspend()
	{
		_conn.onmessage = [](auto &_) {};
		return log("bot has been suspended.");
	}

	Bot &Bot::resume()
	{
		_conn.onmessage = [this](const std::string &packet)
		{
			if (_state == WORK)
				this->on_message(packet);
		};

		return log("bot has been resumed.");
	}

	Bot &Bot::flush_all()
	{
		auto &red = _redinfo;
		red.err->flush();
		red.log->flush();
		red.dbg->flush();
		return *this;
	}

	Bot &Bot::permit(ServiceAlias service)
	{
		if (_services.contains(service))
			_services.at(service)->enable(*this);
		return *this;
	}

	Bot &Bot::forbid(ServiceAlias service)
	{
		if (_services.contains(service))
			_services.at(service)->disable(*this);
		return *this;
	}

	Bot &Bot::subscribe(ServiceAlias service)
	{
		if (_home->available().contains(service))
			if (not _services.contains(service))
			{
				log("start subscribing service: {}.", service);
				_services.emplace(service, _home->available().at(service));
				auto new_service = _services.at(service);
				new_service->install(*this);
				log("service: {} subscription completed.", service);
			}
		return *this;
	}

	Bot &Bot::unsubscribe(ServiceAlias service)
	{
		if (_home->available().contains(service))
			if (_services.contains(service))
			{
				log("start unsubscribing service: {}.", service);
				auto new_service = _services.at(service);
				new_service->uninstall(*this);
				_services.erase(service);
				log("service: {} unsubscription completed.", service);
			}
		return *this;
	}

	TimerTrace Bot::set_timeout(int timeout_ms, hv::TimerCallback callback)
	{
		auto loop = _home->thread_pool().nextLoop();
		auto timer_id = loop->setTimeout(timeout_ms, callback);
		return std::make_tuple(loop, timer_id);
	}

	TimerTrace Bot::set_interval(int interval_ms, hv::TimerCallback callback)
	{
		auto loop = _home->thread_pool().nextLoop();
		auto timer_id = loop->setInterval(interval_ms, callback);
		return std::make_tuple(loop, timer_id);
	}

	void Bot::kill_timer(const TimerTrace &trace)
	{
		auto &[loop, timer_id] = trace;
		loop->killTimer(timer_id);
	}

	void Bot::reset_timer(const TimerTrace &trace)
	{
		auto &[loop, timer_id] = trace;
		loop->resetTimer(timer_id);
	}

	Qid Bot::id()
	{
		static std::hash<std::string_view> hasher{};
		return hasher(name());
	}

	ApiCall Bot::active()
	{
		return ApiCall::from(shared_from_this());
	}

	ApiWarpper Bot::api()
	{
		return ApiWarpper::from(shared_from_this());
	}

	void Bot::on_message(const std::string &full_packet)
	{
		json packet = json::parse(full_packet);
		if (packet.contains("retcode")) // onebot-api call result
		{
			auto ecode = packet["echo"].get<int>();
			if (ecode != ApiCall::IgnoreResultFlag)
				if ("ok" == packet["status"].get<std::string>())
				{
					_home->thread_pool()
						.nextLoop()
						->queueInLoop(
							[data = std::move(packet["data"]), hook = std::move(_hooks.at(ecode))]()
							{ std::invoke(hook, data); });
					_hooks.erase(ecode);
				}
		}
		else if (packet.contains("post_type")) /// event post
		{
			Event event;
			try
			{
				event = EventParser::parse(packet);
			}
			catch (const std::out_of_range &out)
			{
				err_polish<std::logic_error>(out,
											 "Event Parser",
											 fmt::format("Unsupported Event from packet: {}",
														 packet.dump()));
			}
			catch (const std::exception &e)
			{
				extern_err_report<std::runtime_error>("Parse Event Failed", e.what());
			}

			try
			{
				response(std::move(event));
			}
			catch (const std::exception &e)
			{
				err_polish(e, "Response Event Failed", "");
			}
		}
	}

	Bot &Bot::response(Event &&event)
	{
		OptEvent opteve{std::forward<Event>(event)};
		bool handled{false};
		for (auto &[_, sptr] : _services)
		{
			try
			{
				sptr->serve(*this, opteve);
			}
			catch (const std::exception &e)
			{
				throw std::runtime_error(fmt::format("Failed Service '{0}', Error: {1}", sptr->name(), e.what()));
			}
			if (not opteve.has_value())
			{
				handled = true;
				break;
			}
		}
		if (not handled)
		{
			static EventRoughDumper dumper{};
			log("ignored event: {}", std::visit(dumper, opteve.value()));
		}

		return *this;
	}

	Bot &Bot::set_hook(int ecode, std::function<void(const json &)> &&hook, unsigned timeout_ms)
	{
		_hooks.emplace(ecode, std::forward<std::function<void(const json &)>>(hook));
		_home->thread_pool()
			.nextLoop()
			->setTimeout(timeout_ms, [=](hv::TimerID _)
						 { _hooks.erase(ecode); });
		return *this;
	}

	std::string Bot::_log_impl(const char *level, std::string_view content)
	{
		return fmt::format(log_fmt, time_now(), _self, level, content);
	}

	Env &Bot::home()
	{
		return *_home;
	}

	std::shared_ptr<hv::WebSocketChannel> &Bot::channel()
	{
		return _conn.channel;
	}

	RedirectInfo Bot::redirect_info() const
	{
		return _redinfo;
	}

	const std::set<Qid> &Bot::admins() const
	{
		return _admins;
	}

	std::string_view Bot::name()
	{
		return _self;
	}
}
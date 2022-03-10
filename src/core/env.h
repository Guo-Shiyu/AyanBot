#pragma once

#include "fwd.h"
#include "redirect.h"

#include "hv/EventLoopThreadPool.h"

#include <optional>
#include <map>
#include <set>

namespace ayan
{
	template <class C>
	concept IterableQidContainer = requires(C container)
	{
		requires Iterable<C>;
		{std::is_same<decltype(*std::begin(container)), Qid>()};
	};

	class EnvBuilder
	{
	public:
		template <IterableQidContainer R>
		EnvBuilder &with_admins(R &&admins)
		{
			for (auto qid : admins)
				_admins.insert(qid);
			return *this;
		}

		EnvBuilder &with_name(const std::string &name);
		EnvBuilder &with_admin(Qid admin_id);
		EnvBuilder &with_thread_num(unsigned int num);
		EnvBuilder &redirect_log(std::ostream *log);
		EnvBuilder &redirect_err(std::ostream *err);
		EnvBuilder &redirect_dbg(std::ostream *dbg);
		std::shared_ptr<Env> init();

	private:
		std::string _name = "Global";
		unsigned int _tpsize = 1;
		RedirectInfo _redinfo = RedirectInfo::Default();
		std::set<Qid> _admins = {};
	};

	class Env : public std::enable_shared_from_this<Env>
	{
	public:
		~Env();
		Env() = delete;
		Env(const Env &) = delete;
		explicit Env(std::string &&name, unsigned tpsize = 1U, RedirectInfo redinfo = RedirectInfo::Default(), std::optional<std::set<Qid>> &&superuser = std::nullopt);

	public:
		static EnvBuilder from();

	public:
		template <typename S, bool Required = false, typename... Args>
		Env &supply(std::optional<std::string_view> alias = std::nullopt, Args... args)
		{
			auto service = new S(args...);
			auto name = alias.value_or(service->name());
			_supply.insert_or_assign(name, reinterpret_cast<ServiceConcept *>(service));
			if (Required)
				_required.insert(name);
			return *this;
		}

		Env &registry(const BotPtr &bot);

		// size_t remove_superuser(Qid id);

	public:
		RedirectInfo redirect_info() const;
		const std::map<std::string_view, ServiceConcept *> &available() const;
		const std::set<std::string_view> &required() const;
		const std::set<Qid> &superusers() const;
		const std::map<Qid, BotPtr> &bots() const;
		hv::EventLoopThreadPool &thread_pool();
		std::string_view name() const;

	private:
		const std::string _name;
		RedirectInfo _redinfo;
		std::set<Qid> _superuser;
		hv::EventLoopThreadPool _tp;
		std::map<std::string_view, ServiceConcept *> _supply;
		std::set<std::string_view> _required;
		std::map<Qid, BotPtr> _bot;
	};
}
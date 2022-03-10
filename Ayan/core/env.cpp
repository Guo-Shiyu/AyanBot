#include "env.h"
#include "service.h"

namespace ayan
{
	EnvBuilder &EnvBuilder::with_thread_num(unsigned int num)
	{
		_tpsize = num;
		return *this;
	}

	EnvBuilder &EnvBuilder::with_name(const std::string &name)
	{
		_name = name;
		return *this;
	}

	EnvBuilder &EnvBuilder::with_admin(Qid admin_id)
	{
		_admins.insert(admin_id);
		return *this;
	}

	EnvBuilder &EnvBuilder::redirect_log(std::ostream *log)
	{
		_redinfo.log = log;
		return *this;
	}

	EnvBuilder &EnvBuilder::redirect_err(std::ostream *err)
	{
		_redinfo.err = err;
		return *this;
	}

	EnvBuilder &EnvBuilder::redirect_dbg(std::ostream *dbg)
	{
		_redinfo.dbg = dbg;
		return *this;
	}

	std::shared_ptr<Env> EnvBuilder::init()
	{
		auto env = std::make_shared<Env>(std::move(_name), _tpsize, _redinfo, _admins);
		env->thread_pool().start();
		return env;
	}

	Env::~Env()
	{
		for (auto &[_, bot] : _bot)
			bot->stop();
		for (auto &[_, service_ptr] : _supply)
			delete service_ptr;
	}

	Env::Env(std::string &&name, unsigned tpsize, RedirectInfo redinfo, std::optional<std::set<Qid>> &&superuser)
		: _name(std::forward<std::string>(name)), _redinfo(redinfo), _superuser(superuser.value_or(decltype(_superuser){})), _tp(tpsize), _supply(), _required(), _bot()
	{
	}

	EnvBuilder Env::from()
	{
		return EnvBuilder{};
	}

	Env &Env::registry(const BotPtr &bot)
	{
		_bot.emplace(bot->id(), bot);
		return *this;
	}

	RedirectInfo Env::redirect_info() const
	{
		return _redinfo;
	}

	const std::map<std::string_view, ServiceConcept *> &Env::available() const
	{
		return _supply;
	}

	const std::set<std::string_view> &Env::required() const
	{
		return _required;
	}

	const std::map<Qid, BotPtr> &Env::bots() const
	{
		return _bot;
	}

	hv::EventLoopThreadPool &Env::thread_pool()
	{
		return _tp;
	}

	std::string_view Env::name() const
	{
		return _name;
	}

	const std::set<Qid> &Env::superusers() const
	{
		return _superuser;
	}
}
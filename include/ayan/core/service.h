#pragma once

#include "ayan/fwd.h"
#include "ayan/onebot/event.h"
#include "ayan/utils/util.h"

#include <algorithm>
#include <any>

namespace ayan {
class Bot;

namespace service {
using RetCode = int;

struct RunResult {

  RetCode  ret   = NoSignaficantRet; // 返回值
  std::any extra = {};               // 额外信息

  template <typename T>
  Optional<T> as() {
    try {
      return std::any_cast<T>(extra);
    } catch (const std::bad_any_cast &_) { return NullOpt; }
  }

  bool ill() {
    return ret == NoSignaficantRet;
  }

  bool success() {
    return ret > NoSignaficantRet;
  }

  bool failed() {
    return ret < NoSignaficantRet;
  }

  constexpr static RetCode NoSignaficantRet = 0;

  static RunResult nothing() {
    return RunResult{};
  }
};

struct ServiceConcept {
  constexpr virtual std::string_view identity() const noexcept = 0;

  // -> load()
  virtual void install(const Shared<Bot> &bot) noexcept = 0;

  // -> unload()
  virtual void uninstall(const Shared<Bot> &bot) noexcept = 0;

  // -> run()
  virtual RunResult serve(const Shared<Bot> &bot, Event &event) noexcept = 0;
};

using ServicePtr = Shared<ServiceConcept>;

namespace inner {
using SevCtor = std::function<std::shared_ptr<ServiceConcept>(void)>;

template <typename Sev>
struct ServiceCreator {
  static std::shared_ptr<ServiceConcept> create() {
    return std::make_shared<Sev>();
  }
};

using SevSupportMap = std::unordered_map<std::string_view, SevCtor>;

static SevSupportMap &all_available_services() {
  static SevSupportMap map;
  return map;
}

template <typename Sev>
void make_available() {
  auto &map      = all_available_services();
  auto  sev_name = util::type_name<Sev>();
  map.emplace(sev_name, ServiceCreator<Sev>::create);
}

template <typename SevImpl>
struct SevRegister {
  SevRegister() {
    Registered = true;
  }

  static bool Registered;
  static bool registery() {
    make_available<SevImpl>();
    return true;
  }
};

template <typename SevImpl>
bool SevRegister<SevImpl>::Registered = SevRegister<SevImpl>::registery();

} // namespace inner

class ServiceManager {
public:
  struct ExecState {
    RunResult              result;
    Shared<ServiceConcept> sev;
  };

  using Self         = ServiceManager;
  using ExecuteStack = std::map<std::string_view, ExecState>;

public:
  size_t size() const {
    return this->stack_.size();
  }

  Self &require(const std::string &service_name) noexcept {
    auto                   ctor = inner::all_available_services().at(service_name);
    Shared<ServiceConcept> sev  = std::invoke(ctor);

    stack_[sev->identity()] = ExecState{.sev = std::move(sev)};
    return *this;
  }

  template <typename Sev>
  Self &require() noexcept {
    return require(util::type_name<Sev>());
  }

  void remove_all() noexcept {
    stack_.clear();
  }

  bool remove(std::string_view service_name) noexcept {
    return std::erase_if(stack_, [=](const auto &kvp) {
             auto &[_, state] = kvp;
             return service_name == state.sev->identity();
           }) >= 1;
  }

  template <typename Sev>
  bool remove() noexcept {
    return remove(util::type_name<Sev>());
  }

  size_t invalid_if(const std::function<bool(const ExecState &)> &cond) {
    size_t count = 0;
    std::for_each(stack_.begin(), stack_.end(), [&](auto &kvp) {
      auto &[_, state] = kvp;
      if (cond(state)) {
        state.result = RunResult{};
        count        += 1;
      }
    });
    return count;
  };

  size_t invalid_if(const std::function<bool(RetCode)> &cond) {
    return invalid_if([&](const ExecState &s) { return cond(s.result.ret); });
  }

  bool invalid(std::string_view sev_name) noexcept {
    return invalid_if([&](const ExecState &s) { return s.sev->identity() == sev_name; }) >
           0;
  }

  size_t invalid_all() noexcept {
    return invalid_if([](auto &_) { return true; });
  }

  // 使用新的结果替换掉缓存的旧结果
  RunResult replace(ExecuteStack::key_type key, RunResult &&newer) {
    auto ret           = std::move(stack_[key].result);
    stack_[key].result = newer;
    return ret;
  }

  void foreach (const std::function<void(ExecuteStack::value_type &)> &visit) {
    std::for_each(stack_.begin(), stack_.end(), visit);
  };

  void want(const ServicePtr &sev) {
    this->stack_[sev->identity()] = ExecState{.result = RunResult{}, .sev = sev};
  }

private:
  ExecuteStack stack_; // execute stack
};

template <typename Impl>
class ServiceImpl : public ServiceConcept,
                    public inner::SevRegister<Impl>,
                    public std::enable_shared_from_this<Impl> {
public:
  constexpr std::string_view identity() const noexcept override final {
    // static std::string name = util::type_name<Impl>();
    // return name;
    return util::type_name<Impl>();
  }

  ServiceManager &prelude() {
    return this->mgr_;
  }

public:
  /// SIGNATURE:
  /// auto load(Shared<Bot> botptr) -> void;
  /// auto usuage(ServiceManager *super, ServiceManager* self) -> void
  void install(const Shared<Bot> &bot) noexcept override {
    auto impl = static_cast<Impl *>(this);
    impl->usuage(&this->mgr_, &impl->mgr_);
    mgr_.foreach ([&](auto &kvp) {
      auto &[_, state] = kvp;
      state.sev->install(bot);
    });
    impl->load(bot);
  }

  /// SIGNATURE:
  /// auto unload(Shared<Bot> botptr) -> void;
  void uninstall(const Shared<Bot> &bot) noexcept override {
    auto impl = static_cast<Impl *>(this);
    mgr_.foreach ([&](auto &kvp) {
      auto &[_, state] = kvp;
      state.sev->uninstall(bot);
    });

    impl->unload(bot);
  }

  /// SIGNATURE:
  /// auto run(Shared<Bot> botptr, Event &event) -> RunResult;
  RunResult serve(const Shared<Bot> &bot, Event &event) noexcept override {
    auto impl = static_cast<Impl *>(this);
    mgr_.foreach ([&](auto &sep) {
      auto &exec_state  = sep.second;
      exec_state.result = exec_state.sev->serve(bot, event);
    });
    return impl->run(bot, event);
  }

  [[ayan::service_api]] void
  /// 在这个方法中添加自己所需要的各种 service
  usuage(ServiceManager *super, ServiceManager *self) {}

  [[ayan::service_api]] void
  /// 在这个方法中添加自身加载到一个 bot 上的动作
  load(const Shared<Bot> &botptr) {}

  [[ayan::service_api]] void
  /// 在这个方法中添加自身卸载自一个 bot 上的动作
  unload(const Shared<Bot> &botptr) {}

  [[ayan::service_api]] RunResult
  /// 在这个方法中添加自身的处理逻辑
  run(Shared<Bot> botptr, Event &event) {
    return RunResult::nothing();
  }

private:
  ServiceManager mgr_;
};

} // namespace service

using service::RetCode;
using service::RunResult;
using service::ServiceImpl;
using service::ServiceManager;
using service::ServicePtr;

class ServiceSecheduler : public ServiceImpl<ServiceSecheduler> {
public:
  void append(const ServicePtr &sev) {
    prelude().want(sev);
  }

  RunResult serve(const Shared<Bot> &bot, Event &event) noexcept;
};

} // namespace ayan
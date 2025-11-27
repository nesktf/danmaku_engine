#pragma once

#define OKUU_SOL_IMPL
#include "./sol.hpp"

#include "../assets/manager.hpp"
#include "../stage/stage.hpp"

#include <list>

namespace okuu::lua {

struct thread_coro {
public:
  thread_coro(sol::thread&& coro_thread_, sol::coroutine&& coro_);

public:
  static thread_coro from_func(sol::protected_function func);

public:
  template<typename... Args>
  sol::protected_function_result operator()(Args&&... args) {
    return std::invoke(_coro, std::forward<Args>(args)...);
  }

  template<typename... Ret, typename... Args>
  decltype(auto) operator()(sol::types<Ret...> ret, Args&&... args) {
    return std::invoke(_coro, ret, std::forward<Args>(args)...);
  }

private:
  sol::thread _coro_thread;
  sol::coroutine _coro;
};

class stage_env {
public:
  using list_iterator = std::list<sol::protected_function>::iterator;

public:
  stage_env(stage::stage_scene& scene, sol::state&& lua,
            sol::optional<sol::protected_function>&& stage_setup, sol::coroutine&& stage_run);

public:
  static expect<stage_env> load(const std::string& script_path, stage::stage_scene& scene,
                                assets::asset_bundle& assets);

public:
  void setup_stage_modules();
  void run_tasks();

  void trigger_event(std::string name, sol::variadic_args args);
  list_iterator register_event(std::string name, sol::protected_function func);
  void unregister_event(std::string name, list_iterator event);
  void clear_events(std::string name);

  stage::stage_scene& scene() { return *_scene; }

private:
  ntf::weak_ptr<stage::stage_scene> _scene;
  sol::state _lua;
  sol::optional<sol::protected_function> _stage_setup;
  sol::coroutine _stage_run;
  std::unordered_map<std::string, std::list<sol::protected_function>> _events;
};

} // namespace okuu::lua

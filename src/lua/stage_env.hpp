#pragma once

#define OKUU_SOL_IMPL
#include "./sol.hpp"

#include "../assets/manager.hpp"
#include "../stage/stage.hpp"

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
  stage_env(sol::state&& lua, sol::coroutine&& stage_run);

public:
  static expect<stage_env> load(const std::string& script_path, stage::stage_scene& scene,
                                assets::asset_bundle& assets);

public:
  void run_tasks();

private:
  sol::state _lua;
  sol::coroutine _stage_run;
};

} // namespace okuu::lua

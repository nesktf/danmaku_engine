#pragma once

#define OKUU_SOL_IMPL
#include "./sol.hpp"

#include "../core.hpp"

namespace okuu::stage {

class stage_scene;

} // namespace okuu::stage

namespace okuu::lua {

sol::table setup_math_usertypes(sol::state_view lua);

class stage_env {
public:
  stage_env(sol::state lua, sol::thread stage_run_thread, sol::coroutine stage_run);

public:
  static expect<stage_env> load_stage(std::string path);

private:
  sol::state _lua;
  sol::thread _stage_run_thread;
  sol::coroutine _stage_run;
};

class lua_env {
public:
  struct stage_funcs {
    sol::protected_function stage_setup;
    sol::protected_function stage_run;
  };

public:
  lua_env(sol::state&& lua, sol::table lib_table, std::unique_ptr<stage_scene>&& scene);

public:
  static expect<lua_env> load(const std::string& script_path,
                              std::unique_ptr<stage_scene>&& scene);

public:
  static sol::table init_assets(sol::state_view lua, ntf::weak_ptr<stage_scene> scene);
  static sol::table init_package(sol::state_view lua, assets::package_bundle& bundle,
                                 std::vector<player_userdata>& userdatas,
                                 std::vector<stage_entry>& stages,
                                 std::vector<stage_funcs>& stage_funcs);
  static sol::table init_stage(sol::state_view lua, ntf::weak_ptr<stage_scene> scene);
  static sol::table init_util(sol::state_view lua, ntf::weak_ptr<stage_scene> scene);

public:
  void tick();

  stage_scene& scene() { return *_scene; }

  const stage_scene& scene() const { return *_scene; }

private:
  sol::state _lua;

  sol::thread _task_thread;
  sol::coroutine _task;

  u32 _ticks, _frames;
  u32 _task_time;

  std::unique_ptr<stage_scene> _scene;
};

} // namespace okuu::lua

#pragma once

#define OKUU_SOL_IMPL
#include "./sol.hpp"

#include "../assets/manager.hpp"

namespace okuu::stage {

class player_entity;
class boss_entity;
class projectile_entity;

class stage_scene;

using entity_sprite = std::pair<shogle::texture2d_view, render::sprite_uvs>;

} // namespace okuu::stage

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

class lua_player {
public:
  lua_player();

public:
};

class lua_boss {
public:
  lua_boss(u32 slot) noexcept;

public:
  u32 get_slot() const { return _boss_slot; }

  bool is_alive(sol::this_state ts) const;
  void kill(sol::this_state ts);

private:
  u32 _boss_slot;
};

class lua_projectile {
public:
  lua_projectile(u64 handle);

public:
  bool is_alive(sol::this_state ts) const;
  void kill(sol::this_state ts);

private:
  u64 _handle;
};

class lua_stage {
public:
  lua_stage(stage::stage_scene& scene);

public:
  stage::stage_scene& get_scene() { return *_scene; }

public:
  void on_yield(u32 ticks);

  void trigger_dialog(std::string dialog_id);

  lua_player get_player();

  sol::variadic_results get_boss(sol::this_state ts, u32 slot);

  sol::variadic_results spawn_proj(sol::this_state ts, sol::table args);

  sol::table spawn_proj_n(sol::this_state ts, u32 count, sol::protected_function func);

private:
  ntf::weak_ptr<stage::stage_scene> _scene;
};

class lua_sprite {
public:
  stage::entity_sprite get();
};

class lua_asset_bundle {
public:
  lua_asset_bundle(assets::asset_bundle& bundle);

public:
  assets::asset_bundle& get_bundle() { return *_assets; }

private:
  ntf::weak_ptr<assets::asset_bundle> _assets;
};

class stage_env {
public:
  stage_env(sol::state&& lua, sol::thread&& stage_run_thread, sol::coroutine&& stage_run);

public:
  static expect<stage_env> load(const std::string& script_path, stage::stage_scene& scene,
                                assets::asset_bundle& assets);

public:
  void run_tasks();

private:
  sol::state _lua;
  sol::thread _stage_run_thread;
  sol::coroutine _stage_run;
};

} // namespace okuu::lua

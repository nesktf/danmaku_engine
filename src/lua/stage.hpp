#pragma once

#include "../stage/stage.hpp"
#include "./sol.hpp"

namespace okuu::lua {

class lua_player {
public:
  lua_player();

public:
  void kill(sol::this_state ts);
};

class lua_boss {
public:
  lua_boss(u32 slot) noexcept;

public:
  u32 get_slot() const;
  bool is_alive(sol::this_state ts) const;
  void kill(sol::this_state ts) const;

private:
  u32 _boss_slot;
};

class lua_projectile {
public:
  lua_projectile(u64 handle);

public:
  u32 get_handle() const;
  bool is_alive(sol::this_state ts) const;
  void kill(sol::this_state ts) const;
  void set_pos(sol::this_state ts, f32 x, f32 y);
  vec2 get_pos(sol::this_state ts);
  void set_movement(sol::this_state ts, stage::entity_movement movement);

private:
  u64 _handle;
};

class lua_stage {
public:
  lua_stage(stage::stage_scene& scene);

public:
  stage::stage_scene& get() { return *_scene; }

public:
  void on_yield(u32 ticks);

  void trigger_dialog(std::string dialog_id);

  sol::variadic_results get_boss(sol::this_state ts, u32 slot);

  sol::variadic_results spawn_proj(sol::this_state ts, sol::table args);

  sol::table spawn_proj_n(sol::this_state ts, u32 count, sol::protected_function func);

public:
  static sol::table setup_module(sol::table& okuu_lib, stage::stage_scene& scene);
  static stage::stage_scene& instance(sol::state_view lua);

private:
  ntf::weak_ptr<stage::stage_scene> _scene;
};

} // namespace okuu::lua

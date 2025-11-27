#pragma once

#include "../stage/stage.hpp"
#include "./sol.hpp"

#include <list>

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

class lua_event {
public:
  using list_iterator = std::list<sol::protected_function>::iterator;

public:
  lua_event(list_iterator it) : _it{it} {}

public:
  list_iterator get() { return _it; }

private:
  list_iterator _it;
};

class stage_env;

class lua_stage {
public:
  lua_stage(stage_env& env);

public:
  stage_env& get() { return *_env; }

  stage_env* operator->() { return &get(); }

  stage_env& operator*() { return get(); }

public:
  void on_yield(u32 ticks);

  void trigger_event(std::string name, sol::variadic_args args);
  lua_event register_event(std::string name, sol::protected_function func);
  void unregister_event(std::string name, lua_event event);
  void clear_events(std::string name);

  sol::variadic_results get_boss(sol::this_state ts, u32 slot);

  sol::variadic_results spawn_proj(sol::this_state ts, sol::table args);

  sol::table spawn_proj_n(sol::this_state ts, u32 count, sol::protected_function func);

public:
  static lua_stage setup_module(sol::table& okuu_lib, stage_env& env);
  static lua_stage instance(sol::state_view lua);

private:
  ntf::weak_ptr<stage_env> _env;
};

} // namespace okuu::lua

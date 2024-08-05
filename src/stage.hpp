#pragma once

#include <sol/sol.hpp>

#include "core.hpp"

#include "entity/projectile.hpp"
#include "entity/boss.hpp"
#include "entity/player.hpp"

namespace stage {

class stage_state {
public:
  stage_state() = default;

public:
  void load_env(std::string_view stage_script);
  void tick();
  
public:
  uint frames() const { return _frames; }

public:
  vector<entity::projectile> projectiles;
  entity::boss boss;
  entity::player player;

private:
  void _prepare_lua_env();
  void _clean_oob();

private:
  uint _frames{0};
  sol::state _lua;
  sol::protected_function _main_task;
  uint _task_time{0}, _task_wait{0};
};

} // namespace stage

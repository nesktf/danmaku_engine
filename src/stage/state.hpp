#pragma once

#include <sol/sol.hpp>

#include "global.hpp"

#include "stage/projectile.hpp"
#include "stage/boss.hpp"
#include "stage/player.hpp"

namespace stage {

class state {
public:
  state(std::string_view stage_script);

public:
  void tick();

public:
  frames ticks() const { return _tick_count; }

public:
  std::vector<projectile_entity> projectiles;
  boss_entity boss;
  player_entity player;

private:
  void _setup_lua_env();
  void _prepare_player();
  void _clean_oob();

private:
  frames _tick_count{0};
  frame_delay _task_time{0}, _task_wait{0};

  sol::state _lua;
  sol::protected_function _entrypoint;
};

} // namespace stage

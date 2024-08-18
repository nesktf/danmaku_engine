#pragma once

#include "script/lua_env.hpp"

#include "entity/projectile.hpp"
#include "entity/boss.hpp"
#include "entity/player.hpp"

namespace stage {

class state {
public:
  state(std::string_view stage_script);

public:
  void tick();

public:
  frames ticks() const { return _tick_count; }

public:
  std::vector<entity::projectile> projectiles;
  entity::boss boss;
  entity::player player;

private:
  void _clean_oob();

private:
  script::lua_env _lua;
  frames _tick_count{0};
  frame_delay _task_time{0}, _task_wait{0};
};

} // namespace stage

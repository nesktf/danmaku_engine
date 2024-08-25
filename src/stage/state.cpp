#include "stage/state.hpp"
#include "resources.hpp"

namespace stage {

state::state(std::string_view stage_script) : _lua(this, stage_script) {
  player.set_pos((vec2)VIEWPORT*0.5f);
  player.set_scale(40.0f);
  player.set_sprite(res::sprite{0, 0});
}

void state::state::tick() {
  ++_tick_count;

  if (_task_time >= _task_wait) {
    _task_time = 0;

    const auto frame_delay = _lua.call_task();
    if (frame_delay < 0) {
      ntf::log::debug("[stage::state] Lua main task returned");
    } else {
      _task_wait = frame_delay;
    }
  }
  _task_time++;

  for (auto& projectile : projectiles) {
    projectile.tick();
  }

  if (boss.ready()) {
    boss.tick();
  }
  player.tick();

  _clean_oob();
}

void stage::state::_clean_oob() {
  const float extra = 10.f;
  std::erase_if(projectiles, [&](auto& projectile) { 
    const auto pos = projectile.transform().pos();
    return (pos.x < -extra || pos.y < -extra || pos.x > VIEWPORT.x+extra || pos.y > VIEWPORT.y+extra);
  });
}

} // namespace stage

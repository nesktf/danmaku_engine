#include "stage/state.hpp"
#include "resources.hpp"

namespace stage {

state::state(std::string_view stage_script) : _lua(this, stage_script) {
  player.set_pos((vec2)VIEWPORT*0.5f);
  player.set_scale(60.0f);
  auto atlas = res::get_atlas("chara_cirno").value();
  player.set_sprite(atlas, {
    atlas->find_sequence("cirno.idle").value(),
    atlas->find_sequence("cirno.left").value(),
    atlas->find_sequence("cirno.right").value()
  });
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

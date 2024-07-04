#pragma once

#include <shogle/shogle.hpp>
#include <shogle/res/spritesheet.hpp>

#include <shogle/res/shader/sprite.hpp>

#include <cstdlib>

namespace ntf {

inline void player_mover(shogle::window& win, shogle::transform2d& transform, float dt) {
  float speed = 380.0f*dt;
  if (win.get_key(shogle::key_l)) {
    speed *= 0.66f;
  }
  vec2 vel {0.0f};
  if (win.get_key(shogle::key_a)) {
    vel.x = -1.0f;
  } else if (win.get_key(shogle::key_d)) {
    vel.x = 1.0f;
  }
  if (win.get_key(shogle::key_w)) {
    vel.y = -1.0f;
  } else if (win.get_key(shogle::key_s)) {
    vel.y = 1.0f;
  }
  if (glm::length(vel) > 0) {
    vel = speed*glm::normalize(vel);
  }
  transform.set_pos(transform.pos() + vel);
}

} // namespace ntf

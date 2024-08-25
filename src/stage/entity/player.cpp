#include "stage/entity/player.hpp"
#include "input.hpp"
#include "math.hpp"

namespace entity {

void player::tick() {
  cmplx vel{0.0f};
  auto speed = speed_factor;
  if (input::poll_key(input::keycode::key_l)) {
    speed *= 0.66f;
  }

  if (input::poll_key(input::keycode::key_a)) {
    vel.real(-1.0f);
  } else if (input::poll_key(input::keycode::key_d)) {
    vel.real(1.0f);
  }

  if (input::poll_key(input::keycode::key_w)) {
    vel.imag(-1.0f);
  } else if (input::poll_key(input::keycode::key_s)) {
    vel.imag(1.0f);
  }

  if (math::norm2(vel) > 0) { 
    vel = math::normalize(vel);
  }

  const auto pos = transf.cpos() + vel*speed*DT;
  transf.set_pos(glm::clamp(math::conv(pos), vec2{0.0f}, (vec2)VIEWPORT));
}

} // namespace entity

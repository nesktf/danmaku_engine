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

  anim_state next_state = IDLE;
  if (vel.real() > 0) {
    next_state = RIGHT;
  } else if (vel.real() < 0) {
    next_state = LEFT;
  }

  const auto pos = transf.cpos() + vel*speed*DT;
  transf.set_pos(glm::clamp(math::conv(pos), vec2{0.0f}, (vec2)VIEWPORT));

  switch (_state) {
    case IDLE: {
      if (next_state == LEFT) {
        _state = LEFT;
        _animator.hard_switch(_anim[LEFT], 0);
      } else if (next_state == RIGHT) {
        _state = RIGHT;
        _animator.hard_switch(_anim[RIGHT], 0);
      }
      break;
    }
    case RIGHT: {
      if (next_state == IDLE) {
        _state = IDLE;
        _animator.hard_switch(_anim[IDLE], 0);
      } else if (next_state == LEFT) {
        _state = LEFT;
        _animator.hard_switch(_anim[LEFT], 0);
      }
      break;
    }
    case LEFT: {
      if (next_state == IDLE) {
        _state = IDLE;
        _animator.hard_switch(_anim[IDLE], 0);
      } else if (next_state == RIGHT) {
        _state = RIGHT;
        _animator.hard_switch(_anim[RIGHT], 0);
      }
      break;
    }
  }
  _animator.tick();
}

} // namespace entity

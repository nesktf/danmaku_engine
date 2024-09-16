#include "stage/player.hpp"

#include "input.hpp"

namespace stage {

void player_entity::movement_type::tick(ntf::transform2d& transform) {
  cmplx move_dir{0.f};

  if (input::poll_key(input::keycode::key_a)) {
    move_dir.real(-1.f);
  } else if (input::poll_key(input::keycode::key_d)) {
    move_dir.real(1.f);
  }

  if (input::poll_key(input::keycode::key_w)) {
    move_dir.imag(-1.f);
  } else if (input::poll_key(input::keycode::key_s)) {
    move_dir.imag(1.f);
  }

  real norm2 = math::norm2(move_dir);
  if (norm2 > 0) {
    move_dir /= glm::sqrt(norm2);
  }

  if (input::poll_key(input::keycode::key_l)) {
    _vel = move_dir*slow_speed;
  } else {
    _vel = move_dir*base_speed;
  }

  const vec2 clamp_min{0.f};
  const vec2 clamp_max{VIEWPORT};
  transform.set_pos(glm::clamp(math::conv(transform.cpos() + _vel), clamp_min, clamp_max));
}

void player_entity::animator_type::tick(const movement_type& movement) {
  const auto vel = movement.vel();
  animation_state next_state = IDLE;
  if (vel.real() > 0) {
    next_state = RIGHT;
  } else if (vel.real() < 0) {
    next_state = LEFT;
  }

  switch (_state) {
    case IDLE: {
      if (next_state == LEFT) {
        _animator.hard_switch(_anim[IDLE_TO_LEFT], 1);
        _animator.enqueue_sequence(_anim[LEFT], 0);
      } else if (next_state == RIGHT) {
        _animator.hard_switch(_anim[IDLE_TO_RIGHT], 1);
        _animator.enqueue_sequence(_anim[RIGHT], 0);
      }
      break;
    }
    case RIGHT: {
      if (next_state == IDLE) {
        _animator.hard_switch(_anim[RIGHT_TO_IDLE], 1);
        _animator.enqueue_sequence(_anim[IDLE], 0);
      } else if (next_state == LEFT) {
        _animator.hard_switch(_anim[RIGHT_TO_IDLE], 1);
        _animator.enqueue_sequence(_anim[IDLE_TO_LEFT], 1);
        _animator.enqueue_sequence(_anim[LEFT], 0);
      }
      break;
    }
    case LEFT: {
      if (next_state == IDLE) {
        _animator.hard_switch(_anim[LEFT_TO_IDLE], 1);
        _animator.enqueue_sequence(_anim[IDLE], 0);
      } else if (next_state == RIGHT) {
        _animator.hard_switch(_anim[LEFT_TO_IDLE], 1);
        _animator.enqueue_sequence(_anim[IDLE_TO_RIGHT], 1);
        _animator.enqueue_sequence(_anim[RIGHT], 0);
      }
      break;
    }
    default: break;
  }
  _state = next_state;
  _animator.tick();
}

void player_entity::animator_type::set_data(res::atlas atlas, animation_data data) {
  _anim = data;
  _animator = res::sprite_animator{atlas, _anim[0]};
}

res::sprite player_entity::animator_type::sprite() const {
  return res::sprite{_animator.atlas(), _animator.frame()};
}

void player_entity::tick() {
  movement.tick(transform);
  animator.tick(movement);
}

} // namespace stage

#include "game/entity.hpp"

#include "input.hpp"

namespace game::entity {

void movement::tick(ntf::transform2d& transform) {
  // const cmplx v0 = vel;
  cmplx pos = transform.cpos();

  pos += vel;
  vel = acc + ret*vel;

  if (attr != cmplx{}) {
    const cmplx av = attr_p - pos;
    if (attr_exp == 1) {
      vel += attr*av;
    } else {
      real norm2 = math::norm2(av);
      norm2 = std::pow(norm2, attr_exp - .5f);
      vel += attr + (av*norm2);
    }
  }

  transform.set_pos(pos);
}


void animator::tick(frames entity_ticks) {
  if (!use_sequence) {
    return;
  }

  const auto& seq = handle->sequence_at(sequence);
  index = seq[entity_ticks%seq.size()];
}

res::sprite animator::sprite() const {
  return res::sprite{handle, index};
}


animator animator_static(res::sprite sprite) {
  auto [atlas, index] = sprite;
  return animator {
    .handle = atlas,
    .index = index,
    .use_sequence = false,
  };
}

animator animator_sequence(res::sequence_pair sequence) {
  auto [atlas, seq] = sequence;
  return animator {
    .handle = atlas,
    .sequence = seq,
    .use_sequence = true,
  };
}

movement movement_linear(cmplx vel) {
  return movement {
    .vel = vel,
    .acc = 0,
    .ret = 1,
  };
}

movement movement_interp(cmplx vel0, cmplx vel1, real ret) {
  return movement {
    .vel = vel0,
    .acc = vel1*(1-ret),
    .ret = ret,
  };
}

movement movement_interp_hl(cmplx vel0, cmplx vel1, real hl) {
  return movement_interp(vel0, vel1, std::exp2(-1.f/hl));
}

movement movement_interp_simple(cmplx vel, real boost) {
  return movement_interp(vel*(1+boost), vel, .8f);
}

movement movement_towards(cmplx target, cmplx vel, cmplx attr,
                                        real ret) {
  return movement {
    .vel = vel,
    .ret = ret,
    .attr = attr,
    .attr_p = target,
    .attr_exp = 1,
  };
}

void player::movement_type::tick(ntf::transform2d& transform) {
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

void player::animator_type::tick(const movement_type& movement) {
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

void player::animator_type::set_data(res::atlas atlas, animation_data data) {
  _anim = data;
  _animator = res::sprite_animator{atlas, _anim[0]};
}

res::sprite player::animator_type::sprite() const {
  return res::sprite{_animator.atlas(), _animator.frame()};
}

void player::tick() {
  movement.tick(transform);
  animator.tick(movement);
}

void projectile::tick() {
  movement.tick(transform);
  transform.set_rot(transform.rot() + angular_speed);

  animator.tick(_lifetime);

  _lifetime++;
}

void boss::tick() {
  movement.tick(transform);
  animator.tick(_lifetime);

  _lifetime++;
}

} // namespace game::entity

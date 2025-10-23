#include "./entity.hpp"

namespace okuu::stage {

entity_movement::entity_movement(vec2 vel, vec2 acc, real ret) noexcept :
    _vel{vel.x, vel.y}, _acc{acc.x, acc.y}, _ret{ret}, _attr{}, _attr_p{}, _attr_exp{} {}

entity_movement::entity_movement(vec2 vel, vec2 acc, real ret, vec2 attr, vec2 attr_p,
                                 real attr_exp) noexcept :
    _vel{vel.x, vel.y}, _acc{acc.x, acc.y}, _ret{ret}, _attr{attr.x, attr.y},
    _attr_p{attr_p.x, attr_p.y}, _attr_exp{attr_exp} {}

void entity_movement::next_pos(vec2& curr_pos) {
  cmplx pos{curr_pos.x, curr_pos.y};
  pos += _vel;
  _vel = _acc + _ret * _vel;

  if (_attr != cmplx{}) {
    const cmplx av = _attr_p - pos;
    if (_attr_exp == 1) {
      _vel += _attr * av;
    } else {
      real norm2 = (av.real() * av.real()) + (av.imag() * av.imag());
      norm2 = std::pow(norm2, _attr_exp - .5f);
      _vel += _attr + (av * norm2);
    }
  }
  curr_pos.x = pos.real();
  curr_pos.y = pos.imag();
}

entity_movement entity_movement::move_linear(vec2 vel) {
  return {vel, vec2{0.f}, 1.f};
}

entity_movement entity_movement::move_interpolated(vec2 vel0, vec2 vel1, real ret) {
  const vec2 end_vel = vel1 * (1.f - ret);
  return {vel0, end_vel, ret};
}

entity_movement entity_movement::move_interpolated_halflife(vec2 vel0, vec2 vel1, real hl) {
  return move_interpolated(vel0, vel1, std::exp2(-1.f / hl));
}

entity_movement entity_movement::move_interplated_simple(vec2 vel, real boost) {
  return move_interpolated(vel * (1.f + boost), vel, .8f);
}

entity_movement entity_movement::move_towards(vec2 target, vec2 vel, vec2 attr, real ret) {
  return {vel, vec2{0.f}, ret, attr, target, 1.f};
}

void player_entity::tick() {
  ++_ticks;
  _movement.next_pos(_pos);
}

} // namespace okuu::stage

namespace stage {

void entity_animator::tick(frames entity_ticks) {
  if (!use_sequence) {
    return;
  }

  const auto& seq = handle->sequence_at(sequence);
  index = seq[entity_ticks % seq.size()];
}

res::sprite entity_animator::sprite() const {
  return res::sprite{handle, index};
}

entity_animator entity_animator_static(res::atlas atlas, res::atlas_type::texture_handle index) {
  return entity_animator{
    .handle = atlas,
    .index = index,
    .use_sequence = false,
  };
}

entity_animator entity_animator_sequence(res::atlas atlas, res::atlas_type::sequence_handle seq) {
  return entity_animator{
    .handle = atlas,
    .sequence = seq,
    .use_sequence = true,
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
    _vel = move_dir * slow_speed;
  } else {
    _vel = move_dir * base_speed;
  }

  const vec2 clamp_min{0.f};
  const vec2 clamp_max{VIEWPORT};
  transform.pos(glm::clamp(math::conv(transform.cpos() + _vel), clamp_min, clamp_max));
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
    default:
      break;
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
  transform.rot(transform.rot() + angular_speed);

  animator.tick(_lifetime);

  _lifetime++;
}

void boss::tick() {
  movement.tick(transform);
  animator.tick(_lifetime);

  _lifetime++;
}

} // namespace stage

#include "./entity.hpp"
#include "../render/instance.hpp"

namespace okuu::stage {

entity_movement::entity_movement() noexcept :
    _vel{}, _acc{}, _ret{}, _attr{}, _attr_p{}, _attr_exp{} {}

entity_movement::entity_movement(vec2 vel, vec2 acc, real ret) noexcept :
    _vel{vel.x, vel.y}, _acc{acc.x, acc.y}, _ret{ret}, _attr{}, _attr_p{}, _attr_exp{} {}

entity_movement::entity_movement(vec2 vel, vec2 acc, real ret, vec2 attr, vec2 attr_p,
                                 real attr_exp) noexcept :
    _vel{vel.x, vel.y},
    _acc{acc.x, acc.y}, _ret{ret}, _attr{attr.x, attr.y}, _attr_p{attr_p.x, attr_p.y},
    _attr_exp{attr_exp} {}

void entity_movement::next_pos(vec2& curr_pos) {
  cmplx pos{curr_pos.x, curr_pos.y};
  pos += _vel;
  _vel = _acc + (_ret * _vel);

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

projectile_entity::projectile_entity(u32 birth, vec2 pos, vec2 scale, real angular_speed,
                                     entity_sprite sprite, entity_movement movement) :
    _birth{birth},
    _ticks{0}, _pos{pos}, _scale{scale}, _rot{0.f}, _angular_speed{angular_speed}, _flags{0},
    _movement{movement}, _sprite{sprite} {}

void projectile_entity::tick() {
  _movement.next_pos(_pos);
  _rot += _angular_speed;
  ++_ticks;
}

mat4 projectile_entity::transform() const {
  shogle::basic_transform<real, shogle::trs_transform<real, 2>, 2, true> t{};
  t.pos(_pos).scale(_scale).rot(vec3{0.f, 0.f, _rot});
  mat4 mat = t.world();
  return mat;
}

boss_entity::boss_entity(u32 birth, vec2 pos, entity_sprite sprite, entity_movement movement) :
    _birth{birth}, _ticks{0}, _pos{pos}, _movement{movement}, _flags{0}, _sprite{sprite} {}

void boss_entity::tick() {
  _movement.next_pos(_pos);
  ++_ticks;
}

mat4 boss_entity::transform() const {
  shogle::transform2d<real> t{};
  t.pos(_pos).scale(50.f);
  mat4 mat = t.world();
  return mat;
}

entity_sprite boss_entity::sprite() const {
  return _sprite;
}

player_entity::player_entity(vec2 pos, animation_data&& anims,
                             assets::sprite_animator&& animator) :
    _ticks{0},
    _pos{pos}, _vel{}, _flags{0}, _animator{std::move(animator)},
    _anim_state{animation_state::IDLE}, _anims{std::move(anims)} {}

void player_entity::tick() {
  cmplx move_dir{0.f};
  const auto& win = render::window();

  {
    if (win.poll_key(shogle::win_key::a) == shogle::win_action::press) {
      move_dir.real(-1.f);
    } else if (win.poll_key(shogle::win_key::d) == shogle::win_action::press) {
      move_dir.real(1.f);
    }

    if (win.poll_key(shogle::win_key::w) == shogle::win_action::press) {
      move_dir.imag(-1.f);
    } else if (win.poll_key(shogle::win_key::s) == shogle::win_action::press) {
      move_dir.imag(1.f);
    }

    real norm2 = shogle::norm2(move_dir);
    if (norm2 > 0) {
      move_dir /= glm::sqrt(norm2);
    }

    auto vel = _vel;
    vel.x = move_dir.real();
    vel.y = move_dir.imag();

    const real slow_speed = .66f;
    if (win.poll_key(shogle::win_key::l) == shogle::win_action::press) {
      vel.x *= slow_speed;
      vel.y *= slow_speed;
    }

    // const vec2 clamp_min{0.f};
    // const vec2 clamp_max{VIEWPORT};
    _vel = vel;
    _pos += vel * 8.f;
  }

  animation_state next_state = IDLE;
  if (_vel.x > 0.f) {
    next_state = RIGHT;
  } else if (_vel.x < 0.f) {
    next_state = LEFT;
  }

  switch (_anim_state) {
    case IDLE: {
      if (next_state == LEFT) {
        _animator.hard_switch(_anims[IDLE_TO_LEFT], 1);
        _animator.enqueue(_anims[LEFT], 0);
      } else if (next_state == RIGHT) {
        _animator.hard_switch(_anims[IDLE_TO_RIGHT], 1);
        _animator.enqueue(_anims[RIGHT], 0);
      }
      break;
    }
    case RIGHT: {
      if (next_state == IDLE) {
        _animator.hard_switch(_anims[RIGHT_TO_IDLE], 1);
        _animator.enqueue(_anims[IDLE], 0);
      } else if (next_state == LEFT) {
        _animator.hard_switch(_anims[RIGHT_TO_IDLE], 1);
        _animator.enqueue(_anims[IDLE_TO_LEFT], 1);
        _animator.enqueue(_anims[LEFT], 0);
      }
      break;
    }
    case LEFT: {
      if (next_state == IDLE) {
        _animator.hard_switch(_anims[LEFT_TO_IDLE], 1);
        _animator.enqueue(_anims[IDLE], 0);
      } else if (next_state == RIGHT) {
        _animator.hard_switch(_anims[LEFT_TO_IDLE], 1);
        _animator.enqueue(_anims[IDLE_TO_RIGHT], 1);
        _animator.enqueue(_anims[RIGHT], 0);
      }
      break;
    }
    default:
      break;
  }
  _anim_state = next_state;
  _animator.tick();
  ++_ticks;
}

mat4 player_entity::transform(const render::sprite_uvs& uvs) const {
  shogle::transform2d<real> t{};
  f32 ratio = uvs.x_lin / uvs.y_lin;
  t.pos(_pos).scale(80.f * ratio, 80.f);
  mat4 mat = t.world();
  return mat;
}

entity_sprite player_entity::sprite() const {
  return {_animator.atlas(), _animator.frame()};
}

} // namespace okuu::stage

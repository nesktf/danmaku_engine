#pragma once

#include "../assets/sprite.hpp"
#include <shogle/shogle.hpp>

namespace okuu::stage {

using namespace ntf::numdefs;
using shogle::cmplx;
using shogle::mat4;
using shogle::vec2;
using shogle::vec3;
using real = f32;

using entity_sprite =
  std::pair<ntf::weak_cptr<assets::sprite_atlas>, assets::sprite_atlas::sprite>;

class entity_movement {
private:
  entity_movement(vec2 vel, vec2 acc, real ret) noexcept;
  entity_movement(vec2 vel, vec2 acc, real ret, vec2 attr, vec2 attr_p, real attr_exp) noexcept;

public:
  entity_movement() noexcept;

public:
  static entity_movement move_linear(vec2 vel);
  static entity_movement move_interpolated(vec2 vel0, vec2 vel1, real ret);
  static entity_movement move_interpolated_halflife(vec2 vel0, vec2 vel1, real hl);
  static entity_movement move_interplated_simple(vec2 vel, real boost);
  static entity_movement move_towards(vec2 target, vec2 vel, vec2 attr, real ret);

public:
  void next_pos(vec2& prev_pos);

public:
  vec2 vel() const { return {_vel.real(), _vel.imag()}; }

  entity_movement& vel(real x, real y) {
    _vel.real(x);
    _vel.imag(y);
    return *this;
  }

  vec2 acc() const { return {_acc.real(), _acc.imag()}; }

  entity_movement& acc(real x, real y) {
    _acc.real(x);
    _acc.imag(y);
    return *this;
  }

private:
  cmplx _vel, _acc;
  real _ret;

  cmplx _attr, _attr_p;
  real _attr_exp;
};

class projectile_entity {
public:
  projectile_entity(u32 birth, vec2 pos, vec2 scale, real angular_speed, entity_sprite sprite,
                    entity_movement movement = {});

public:
  void tick();

  mat4 transform() const;

  entity_sprite sprite() const;

  void set_movement(entity_movement movement) { _movement = movement; }

  void set_angular_speed(real speed) { _angular_speed = speed; }

  real angular_speed() const { return _angular_speed; }

private:
  u32 _birth;
  u32 _ticks;
  vec2 _pos;
  vec2 _scale;
  real _rot;
  real _angular_speed;
  u32 _flags;
  entity_movement _movement;
  entity_sprite _sprite;
};

class boss_entity {
public:
  boss_entity(u32 birth, vec2 pos, entity_sprite sprite, entity_movement movement = {});

public:
  vec2 pos() const { return _pos; }

  boss_entity& pos(real x, real y) {
    _pos.x = x;
    _pos.y = y;
    return *this;
  }

  void tick();

  mat4 transform() const;

  entity_sprite sprite() const;

  boss_entity& set_movement(entity_movement movement) {
    _movement = movement;
    return *this;
  }

private:
  u32 _birth;
  u32 _ticks;
  vec2 _pos;
  entity_movement _movement;
  u32 _flags;
  entity_sprite _sprite;
};

class player_entity {
public:
  enum animation_state : u8 {
    IDLE = 0,
    LEFT,
    LEFT_TO_IDLE,
    IDLE_TO_LEFT,
    RIGHT,
    RIGHT_TO_IDLE,
    IDLE_TO_RIGHT,
    ANIM_COUNT,
  };

  using animation_data = std::array<assets::sprite_atlas::animation, ANIM_COUNT>;

public:
  player_entity(vec2 pos, animation_data&& anims, assets::sprite_animator&& animator);

public:
  void tick();

  mat4 transform(const render::sprite_uvs& uvs) const;

  entity_sprite sprite() const;

  vec2 pos() const { return _pos; }

  player_entity& pos(real x, real y) {
    _pos.x = x;
    _pos.y = y;
    return *this;
  }

private:
  u32 _ticks;
  vec2 _pos;
  vec2 _vel;
  u32 _flags;
  assets::sprite_animator _animator;
  animation_state _anim_state;
  animation_data _anims;
};

} // namespace okuu::stage

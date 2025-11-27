#pragma once

#define OKUU_SOL_IMPL
#include "../lua/sol.hpp"

#include "../assets/manager.hpp"
#include <shogle/shogle.hpp>

namespace okuu::stage {

using namespace ntf::numdefs;
using shogle::cmplx;
using shogle::mat4;
using shogle::vec2;
using shogle::vec3;
using real = f32;

using entity_sprite = std::tuple<assets::atlas_handle, assets::sprite_atlas::sprite, vec2>;

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

struct projectile_args {
  vec2 pos;
  vec2 vel;
  vec2 scale;
  real angular_speed;
  entity_sprite sprite;
  entity_movement movement;
  ntf::optional<sol::coroutine> state_handler;
};

class projectile_entity {
public:
  using args_type = projectile_args;

public:
  projectile_entity(projectile_args args);

public:
  void tick();

  mat4 transform(const render::sprite_uvs& uvs) const;

  entity_sprite sprite() const { return _sprite; }

  projectile_entity& movement(entity_movement movement) {
    _movement = movement;
    return *this;
  }

  projectile_entity& angular_speed(real speed) {
    _angular_speed = speed;
    return *this;
  }

  real angular_speed() const { return _angular_speed; }

  vec2 pos() const { return _pos; }

  projectile_entity& pos(f32 x, f32 y) {
    _pos.x = x;
    _pos.y = y;
    return *this;
  }

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
  ntf::optional<sol::coroutine> _state_handler;
};

struct boss_args {
  vec2 pos;
  entity_sprite sprite;
  entity_movement movement;
};

class boss_entity {
public:
  enum boss_flags {
    FLAG_NONE = 0,
    FLAG_ACTIVE = 1 << 0,
  };

public:
  boss_entity();

public:
  boss_entity& setup(const boss_args& args);
  boss_entity& disable();

  bool is_active() const { return _flags & FLAG_ACTIVE; }

public:
  u32& flags() { return _flags; }

  vec2 pos() const { return _pos; }

  boss_entity& pos(real x, real y) {
    _pos.x = x;
    _pos.y = y;
    return *this;
  }

  void tick();

  mat4 transform(const render::sprite_uvs& uvs) const;

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
  ntf::optional<entity_sprite> _sprite;
};

struct sprite_args {
  vec2 pos;
  vec2 scale;
  real rot;
  real angular_speed;
  entity_sprite sprite;
  entity_movement movement;
};

class sprite_entity {
public:
  using args_type = sprite_args;

public:
  sprite_entity(sprite_args args);

public:
  mat4 transform(const render::sprite_uvs& uvs) const;
  void tick();

  vec2 pos() const { return _pos; }

  sprite_entity& pos(real x, real y) {
    _pos.x = x;
    _pos.y = y;
    return *this;
  }

  entity_sprite sprite() const { return _sprite; }

  sprite_entity& set_sprite(entity_sprite sprite) {
    _sprite = sprite;
    return *this;
  }

  sprite_entity& set_movement(entity_movement movement) {
    _movement = movement;
    return *this;
  }

private:
  vec2 _pos;
  vec2 _scale;
  real _rot;
  real _angular_speed;
  entity_sprite _sprite;
  entity_movement _movement;
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

  using anim_pair = std::pair<assets::sprite_atlas::animation, u32>;
  using animation_data = std::array<anim_pair, ANIM_COUNT>;

public:
  player_entity(assets::atlas_handle atlas, vec2 pos, animation_data&& anims,
                assets::sprite_animator&& animator);

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
  assets::atlas_handle _atlas;
  animation_state _anim_state;
  animation_data _anims;
};

} // namespace okuu::stage

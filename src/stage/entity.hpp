#pragma once

#include "okuu.hpp"
#include "resources.hpp"

#include <variant>

namespace okuu {

class entity_movement {
public:
  void tick(ntf::transform2d& transform);

public:
  cmplx vel{}, acc{};
  real ret{};

  cmplx attr{}, attr_p{};
  real attr_exp{};
};

okuu::entity_movement movement_linear(cmplx vel);
okuu::entity_movement movement_interp(cmplx vel0, cmplx vel1, real ret);
okuu::entity_movement movement_interp_hl(cmplx vel0, cmplx vel1, real hl);
okuu::entity_movement movement_interp_simple(cmplx vel, real boost);
okuu::entity_movement movement_towards(cmplx target, cmplx vel, cmplx attr, real ret);


class stateless_animator {
public:
  void tick(frames entity_ticks);

public:
  okuu::sprite sprite() const;

private:
  okuu::resource<okuu::atlas> _atlas;
  okuu::atlas_texture _tex{ntf::atlas_tombstone};
  okuu::atlas_sequence _seq{ntf::atlas_sequence_tombstone};
};

class input_movement {
public:
  input_movement() = default;
  input_movement(real base, real slow) :
    base_speed(base), slow_speed(slow) {}

public:
  void tick(ntf::transform2d& transform);

public:
  cmplx vel() const { return _vel; }
 
private:
  cmplx _vel;

public:
  real base_speed{1.f};
  real slow_speed{1.f};
};

class player_animator {
public:
  enum animation_state : uint8_t {
    IDLE = 0,
    LEFT,
    LEFT_TO_IDLE,
    IDLE_TO_LEFT,
    RIGHT,
    RIGHT_TO_IDLE,
    IDLE_TO_RIGHT,
    ANIM_COUNT,
  };
  using animation_data = std::array<okuu::atlas_sequence, ANIM_COUNT>;

public:
  player_animator() = default;
  player_animator(okuu::resource<okuu::atlas> atlas, animation_data data);

public:
  void tick(const input_movement& movement);
  void set_data(okuu::resource<okuu::atlas> atlas, animation_data data);
  
  okuu::sprite sprite() const;

private:
  okuu::sprite_animator _animator;
  animation_data _anim;
  animation_state _state;
};

okuu::stateless_animator animator_static(okuu::sprite sprite);
okuu::stateless_animator animator_sequence(okuu::sprite_sequence sequence);

class common_entity {
public:
  
private:
  ntf::transform2d _transform;
  okuu::frames _birth, _life{0};
  okuu::entity_movement _movement;
  std::variant<okuu::stateless_animator, okuu::sprite_animator> _anim;
};

class player_entity {

private:
  ntf::transform2d _transform;
  okuu::frames _birth, _life{0};
  okuu::player_animator _anim;
  okuu::input_movement _movement;
};

//
//
// class entity {
// public:
//   entity(frames birth, ntf::transform2d transf = {}, res::sprite sprite = {}) :
//     _transform(std::move(transf)), _sprite(sprite), _birth(birth) {}
//
// public:
//   void transform(ntf::transform2d transf) { _transform = std::move(transf);}
//   void sprite(res::sprite sprite) { _sprite = sprite; }
//
// public:
//   frames birth() const { return _birth; }
//   frames lifetime() const { return _lifetime; }
//
//   const ntf::transform2d& transform() const { return _transform; }
//
//   const mat4& tmat() { return _transform.mat(); }
//   res::sprite sprite() const { return _sprite; }
//
// protected:
//   void _tick_time() { ++_lifetime; }
//
// protected:
//   ntf::transform2d _transform;
//   res::sprite _sprite;
//
// private:
//   frames _birth;
//   frames _lifetime{0};
// };
//
} // namespace okuu

#pragma once

#include "stage/entity.hpp"

namespace stage {

class player_entity {
public:
  class movement_type {
  public:
    movement_type() = default;
    movement_type(real base, real slow) :
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

  class animator_type {
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
    using animation_data = std::array<res::atlas_type::sequence_handle, ANIM_COUNT>;

  public:
    animator_type() = default;
    animator_type(res::atlas atlas, animation_data data) { set_data(atlas, data); }

  public:
    void tick(const movement_type& movement);
    void set_data(res::atlas atlas, animation_data data);
    
    res::sprite sprite() const;

  private:
    res::sprite_animator _animator;
    animation_data _anim;
    animation_state _state;
  };

  struct args {
    ntf::transform2d transform;
    res::atlas atlas;
    animator_type::animation_data anim;
    real base_speed{1.f};
    real slow_speed{1.f};
  };

public:
  player_entity() = default;

  player_entity(args arg) :
    transform(arg.transform),
    movement(arg.base_speed, arg.slow_speed),
    animator(arg.atlas, arg.anim) {}

public:
  void tick();

public:
  res::sprite sprite() const { return animator.sprite(); }
  const mat4& mat() { return transform.mat(); }

public:
  ntf::transform2d transform;

  movement_type movement;
  animator_type animator;
};

} // namespace stage

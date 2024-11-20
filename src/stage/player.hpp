#pragma once

#include "stage/entity.hpp"

namespace okuu::stage {

class player : public entity {
public:
  class movement {
  public:
    movement() = default;
    movement(real base, real slow) :
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

  class animator {
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
    animator() = default;
    animator(res::atlas atlas, animation_data data) { set_data(atlas, data); }

  public:
    void tick(const movement& movement);
    void set_data(res::atlas atlas, animation_data data);
    
    res::sprite sprite() const;

  private:
    res::sprite_animator _animator;
    animation_data _anim;
    animation_state _state;
  };

public:
  player(frames birth, ntf::transform2d transf, real bspd, real sspd, 
         animator::animation_data anim);

public:
  void tick();

public:
  movement move;
  animator anim;
};

} // namespace okuu::stage

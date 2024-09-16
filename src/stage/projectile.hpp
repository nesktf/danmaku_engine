#pragma once

#include "stage/entity.hpp"

namespace stage {

class projectile_entity {
public:
  using movement_type = entity_movement;
  using animator_type = entity_animator;

  using group_type = uint32_t;

  struct args {
    ntf::transform2d transform{};
    group_type group {0};
    movement_type movement;
    animator_type animator;
    real angular_speed {0.f};
  };

public:
  projectile_entity() :
    _birth(global::state().elapsed_ticks) {}

  projectile_entity(args arg) :
    transform(arg.transform),
    movement(arg.movement),
    animator(arg.animator),
    angular_speed(arg.angular_speed),
    _birth(global::state().elapsed_ticks) {}

public:
  void tick();

public:
  frames birth() const { return _birth; }
  frames lifetime() const { return _lifetime; }
  res::sprite sprite() const { return animator.sprite(); }
  const mat4& mat() { return transform.mat(); }

public:
  ntf::transform2d transform;

  movement_type movement;
  animator_type animator;

  real angular_speed;

  group_type group;

private:
  frames _birth;
  frames _lifetime{0};
};

inline void projectile_entity::tick() {
  movement.tick(transform);
  transform.set_rot(transform.rot() + angular_speed);

  animator.tick(_lifetime);

  _lifetime++;
}

} // namespace stage

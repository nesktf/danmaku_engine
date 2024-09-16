#include "stage/entity.hpp"

namespace stage {

class boss_entity {
public:
  using movement_type = entity_movement;
  using animator_type = entity_animator;

  struct args {
    ntf::transform2d transform{};
    movement_type movement{};
    animator_type animator{};
  };

public:
  boss_entity() :
    _birth(global::state().elapsed_ticks) {}

  boss_entity(args arg) :
    transform(arg.transform),
    movement(arg.movement),
    animator(arg.animator),
    hide(false),
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

  bool hide {true};

private:
  frames _birth;
  frames _lifetime{0};
};

inline void boss_entity::tick() {
  movement.tick(transform);
  animator.tick(_lifetime);

  _lifetime++;
}

} // namespace stage

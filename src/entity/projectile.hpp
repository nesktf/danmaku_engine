#pragma once

#include <shogle/scene/transform.hpp>

#include "entity/movement.hpp"
#include "entity/animator.hpp"

namespace ntf {

class projectile {
public:
  projectile(const shogle::sprite* sprite_, entity_movement movement_);

public:
  void tick();

public:
  shogle::transform2d& transf() { return transform; }

private:
  shogle::transform2d transform;
  entity_movement movement;
};

} // namespace ntf::game

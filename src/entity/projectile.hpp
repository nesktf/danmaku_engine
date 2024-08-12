#pragma once

#include "entity/movement.hpp"
#include "resources.hpp"

#include <shogle/scene/transform.hpp>

namespace entity {

class projectile {
public:
  projectile(res::sprite_id sprite_, movement movement_, cmplx init, uint birth_);

public:
  void tick();

public:
  res::sprite_id sprite;
  ntf::transform2d transform;
  movement move;
  uint birth;
};

} // namespace entity

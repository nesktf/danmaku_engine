#pragma once

#include "entity/movement.hpp"

#include <shogle/res/spritesheet.hpp>

namespace entity {

class projectile {
public:
  projectile(ntf::shogle::sprite sprite_, movement movement_, cmplx init, uint birth_);

public:
  void tick();

public:
  ntf::shogle::sprite sprite;
  transform2d transform;
  movement move;
  uint birth;
};

} // namespace entity

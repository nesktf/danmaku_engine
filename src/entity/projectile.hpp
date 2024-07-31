#pragma once

#include "entity/movement.hpp"

#include <shogle/res/spritesheet.hpp>

namespace entity {

class projectile {
public:
  projectile(ntf::shogle::sprite sprite_, movement movement_, uint birth_);

public:
  void tick();

public:
  uint birth;
  ntf::shogle::sprite sprite;
  transform2d transform;
  movement move;
};

} // namespace entity

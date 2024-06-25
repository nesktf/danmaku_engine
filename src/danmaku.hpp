#pragma once

#include "renderer.hpp"
#include <cstdlib>

namespace ntf {

using pos_fun = vec2(*)(vec2, float, float);

struct entity2d {
public:
  entity2d(shogle::sprite* sprite_) :
    sprite(sprite_) {}

public:
  size_t index {0};
  shogle::sprite* sprite{};
  shogle::transform2d transform{};
  vec2 initial_pos{};
  color4 color {1.0f};
  float t{};
  float phase{};

  pos_fun fun{};
};

} // namespace ntf

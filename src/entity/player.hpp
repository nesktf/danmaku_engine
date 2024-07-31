#pragma once

#include "core.hpp"

namespace entity {

class player {
public:
  player() = default;

public:
  void tick();

public:
  cmplx vel;
  // sprite sprite;
  transform2d transform;
};

} // namespace entity

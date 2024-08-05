#pragma once

#include "core.hpp"

namespace entity {

class player {
public:
  player() = default;

public:
  void tick();

public:
  void set_sprite(sprite sp) {
    spr = sp;
    transf.set_scale(spr.sprite_size*scale);
  }

  template<typename Vec>
  void set_pos(Vec vec) {
    transf.set_pos(vec);
  }

  void set_scale(float sc) {
    scale = sc;
  }

  transform2d& transform() { return transf; }

public:
  float scale {40.0f};
  float speed_factor{450.0f};
  sprite spr;
  transform2d transf;
  bool shifting{false};
};

} // namespace entity

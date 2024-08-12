#pragma once

#include "core.hpp"
#include "resources.hpp"

#include <shogle/scene/transform.hpp>

namespace entity {

class player {
public:
  player() = default;

public:
  void tick();

public:
  void set_sprite(res::sprite_id sp) {
    spr = sp;
    const auto& data = res::sprite_data_at(sp);
    transf.set_scale(data.base_size*scale);
  }

  template<typename Vec>
  void set_pos(Vec vec) {
    transf.set_pos(vec);
  }

  void set_scale(float sc) {
    scale = sc;
  }

  ntf::transform2d& transform() { return transf; }

public:
  float scale {40.0f};
  float speed_factor{450.0f};
  res::sprite_id spr;
  ntf::transform2d transf;
  bool shifting{false};
};

} // namespace entity

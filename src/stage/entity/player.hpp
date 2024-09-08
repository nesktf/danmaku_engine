#pragma once

#include "global.hpp"
#include "resources.hpp"

#include "render.hpp"

#include <shogle/scene/transform.hpp>

namespace entity {

class player {
public:
  player() = default;

public:
  void tick();

public:
  void set_sprite(res::sprite sp) {
    _sprite = sp;
    const auto& meta = sp.atlas_handle.get().at(sp.atlas_index);
    transf.set_scale(meta.aspect()*scale);
  }

  template<typename Vec>
  void set_pos(Vec vec) {
    transf.set_pos(vec);
  }

  void set_scale(float sc) {
    scale = sc;
  }

  ntf::transform2d& transform() { return transf; }
  res::sprite sprite() const { return _sprite; }

  const renderer::uniform_tuple& uniforms() const { return _uniforms; }

public:
  res::sprite _sprite;
  ntf::transform2d transf;
  renderer::uniform_tuple _uniforms;

  float scale {40.0f};
  float speed_factor{450.0f};
  bool shifting{false};
};

} // namespace entity

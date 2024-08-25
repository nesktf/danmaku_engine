#pragma once

#include "core.hpp"
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
    const auto& meta = _sprite.meta();
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
  render::shader_renderer* renderer() { return _renderer; }

public:
  res::sprite _sprite;
  ntf::transform2d transf;
  render::shader_renderer* _renderer{nullptr};

  float scale {40.0f};
  float speed_factor{450.0f};
  bool shifting{false};
};

} // namespace entity

#pragma once

#include "core.hpp"
#include "resources.hpp"

#include "render.hpp"
#include "stage/entity/movement.hpp"

#include <shogle/scene/transform.hpp>

namespace entity {

class boss {
public:
  boss() = default;

public:
  void tick() {
    auto new_pos = _transform.cpos();
    _move(new_pos);
    _transform.set_pos(new_pos)
      .set_rot(_transform.rot() + _ang_speed*DT);
  }

public:
  template<typename Vec>
  void init(Vec init_pos, movement first_movement) {
    _ready = true;
    set_pos(init_pos);
    _move = first_movement;
  }

public:
  void set_sprite(res::sprite spr) {
    _sprite = spr;
    const auto& meta = spr.meta();
    _transform.set_scale(meta.aspect()*_scale);
  }

  void set_scale(float scale) {
    _scale = scale;
  }

  void set_angular_speed(float ang_speed) {
    _ang_speed = ang_speed;
  }

  void set_rot(float rot) {
    _transform.set_rot(rot);
  }

  void set_movement(movement move) {
    _move = move;
  }

  template<typename Vec>
  void set_pos(Vec vec) {
    _transform.set_pos(vec);
  }

public:
  ntf::transform2d& transform() { return _transform; }
  res::sprite sprite() { return _sprite; }
  render::shader_renderer* renderer() { return _renderer; }

  bool ready() const { return _ready; }
  
private:
  res::sprite _sprite;
  ntf::transform2d _transform;
  render::shader_renderer* _renderer{nullptr};

  bool _ready {false};
  movement _move;
  float _ang_speed {0.0f};
  float _scale{50.0f};
};

} // namespace entity

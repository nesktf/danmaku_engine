#pragma once

#include "resources.hpp"

#include "render.hpp"
#include "stage/entity/movement.hpp"

#include <shogle/scene/transform.hpp>

namespace entity {

class projectile {
public:
  projectile(res::sprite sprite, movement movement, cmplx init, uint birth) :
    _sprite(sprite), _move(movement), _birth(birth) {
    _transform.set_scale(20.0f).set_pos(init);
  }


public:
  void tick() {
    auto new_pos = _transform.cpos();
    _move(new_pos);
    _transform.set_pos(new_pos)
      .set_rot(_transform.rot() + M_PIf/60);
  }

public:
  ntf::transform2d& transform() { return _transform; }
  res::sprite sprite() const { return _sprite; }
  render::shader_renderer* renderer() { return _renderer; }

  uint birth() { return _birth; }

private:
  res::sprite _sprite;
  ntf::transform2d _transform;
  render::shader_renderer* _renderer{nullptr};

  movement _move;
  uint _birth;
};

} // namespace entity

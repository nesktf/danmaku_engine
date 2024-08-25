#pragma once

#include "core.hpp"

#include "render.hpp"

#include <shogle/scene/transform.hpp>

namespace ui {

class widget {
public:
  virtual void draw(const render::window_data&) = 0;
  virtual void tick() {}

protected:
  ntf::transform2d _transform;
};

} // namespace ui

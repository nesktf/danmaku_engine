#pragma once

#include "okuu.hpp"

namespace okuu::ui {

struct render_data {};

class widget {
public:
  virtual void draw(render_data&) = 0;
  virtual void tick() {}

protected:
  ntf::transform2d _transform;
};

} // namespace ui

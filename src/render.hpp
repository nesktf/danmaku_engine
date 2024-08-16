#pragma once

#include "core.hpp"
#include "resources.hpp"

#include <shogle/scene/transform.hpp>

namespace render {

struct shader_renderer {
public:
  virtual ~shader_renderer() = default;
  virtual void render(res::sprite,ntf::transform2d&,const mat4&,const mat4&) const = 0;

protected:
  res::shader _shader;
};

void init();
void post_init();
void destroy();
void draw(double dt, double alpha);

} // namespace render

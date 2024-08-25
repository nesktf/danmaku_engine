#pragma once

#include "core.hpp"
#include "resources.hpp"

#include <shogle/scene/transform.hpp>

namespace render {

struct window_data {
  mat4 proj, view;
  ivec2 size;
};

struct shader_renderer {
public:
  virtual ~shader_renderer() = default;
  virtual void render(const renderer::texture2d&,ntf::transform2d&,const mat4&,const mat4&) const = 0;

protected:
  res::shader _shader;
};

void init(ntf::glfw::window<renderer>& win);
void post_init(ntf::glfw::window<renderer>& win);
void destroy();
void draw(double dt, double alpha);
void viewport_event(size_t w, size_t h);

} // namespace render

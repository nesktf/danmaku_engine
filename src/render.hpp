#pragma once

#include "resources.hpp"

#include <shogle/scene/transform.hpp>

namespace render {

using window = glfw::window<renderer>;

struct window_data {
  mat4 proj, view;
  ivec2 size;
};

void init(window& win);
void post_init(window& win);
void draw(window& win, double dt, double alpha);
void destroy();

} // namespace render

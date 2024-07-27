#pragma once

#include <shogle/core/types.hpp>

namespace ntf {

void render_init();
void render_new_frame(double dt, double alpha);
void render_update_viewport(size_t w, size_t h);

} // namespace ntf

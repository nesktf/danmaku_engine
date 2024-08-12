#pragma once

#include "core.hpp"

namespace render {

void init();
void post_init();
void destroy();
void draw(double dt, double alpha);

} // namespace render

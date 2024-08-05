#include "global.hpp"

#include "core.hpp"
#include "resources.hpp"
#include "render.hpp"
#include "input.hpp"

static void destroy();
static ntf::cleanup _ {[]{destroy();}};

int main() {
  ntf::log::set_level(ntf::loglevel::verbose);

  render::init();
  res::init();
  global::init();
  input::init();

  render::post_init();

  ntf::shogle::engine_main_loop(UPS, render::draw, global::tick);
}

static void destroy() {
  render::destroy();
}

#include "global.hpp"
#include "resources.hpp"
#include "input.hpp"
#include "render/render.hpp"

static void destroy();
static ntf::cleanup _ {[]{destroy();}};

int main() {
  ntf::log::set_level(ntf::loglevel::verbose);

  render::init();
  res::init();
  global::init();
  input::init();

  render::post_init();

  ntf::engine_main_loop(UPS, render::draw, global::tick);
}

static void destroy() {
  render::destroy();
}

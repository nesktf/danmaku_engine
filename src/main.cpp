#include <shogle/shogle.hpp>

#include "core.hpp"
#include "resources.hpp"
#include "render.hpp"
#include "stage.hpp"
#include "input.hpp"

static void destroy();

static ntf::cleanup _ {[]{destroy();}};

int main() {
  ntf::log::set_level(ntf::loglevel::verbose);

  render::init();
  res::init();
  stage::init();
  input::init();

  render::post_init();

  ntf::shogle::engine_main_loop(UPS, render::draw, stage::tick);
}

static void destroy() {
  render::destroy();
}

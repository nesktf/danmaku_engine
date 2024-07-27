#include <shogle/shogle.hpp>

#include "global.hpp"
#include "resources.hpp"
#include "render.hpp"
#include "stage.hpp"

using namespace ntf;

static ntf::cleanup _ {[]{shogle::engine_destroy();}};

int main() {
  log::set_level(loglevel::verbose);
  shogle::engine_init(WIN_SIZE.x, WIN_SIZE.y, "test");

  resources_init();
  global_init();
  render_init();
  stage_init();

  shogle::engine_viewport_event(render_update_viewport);
  shogle::engine_key_event([](shogle::keycode code, auto, shogle::keystate state, auto) {
    if (code == shogle::key_escape && state == shogle::press) {
      shogle::engine_close_window();
    }
  });

  shogle::engine_main_loop(UPS, render_new_frame, stage_next_tick);
}


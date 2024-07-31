#include "input.hpp"

#include <shogle/engine.hpp>

void input::init() {
  ntf::shogle::engine_key_event([](ntf::shogle::keycode code, auto, ntf::shogle::keystate state, auto) {
    if (code == ntf::shogle::key_escape && state == ntf::shogle::press) {
      ntf::shogle::engine_close_window();
    }
  });
}

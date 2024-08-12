#include "input.hpp"

void input::init() {
  ntf::engine_key_event([](ntf::keycode code, auto, ntf::keystate state, auto) {
    if (code == ntf::key_escape && state == ntf::press) {
      ntf::engine_close_window();
    }
  });
}

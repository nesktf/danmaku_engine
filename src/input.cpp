#include "global.hpp"
#include "input.hpp"

#include "render/ui/frontend.hpp"

#define PRESSED(key) code == key && state == ntf::keystate::press

static void frontend_input(ntf::keycode code, ntf::keystate state) {
  auto& entry = frontend::instance().entry();
  if (PRESSED(ntf::key_s)) {
    entry.set_next_index();
  } else if (PRESSED(ntf::key_w)) {
    entry.set_prev_index();
  }
  if (PRESSED(ntf::key_j)) {
    entry.on_click();
  } else if (PRESSED(ntf::key_k)) {
    frontend::instance().pop();
  }
}

static void gameplay_input(ntf::keycode code, ntf::keystate state) {
  if (PRESSED(ntf::key_i)) {
    global::go_back();
  }
}

void input::init() {
  ntf::engine_key_event([](ntf::keycode code, auto, ntf::keystate state, auto) {
    switch (global::state().current_state) {
      case global::states::loading: {
        break;
      }
      case global::states::frontend: {
        frontend_input(code, state);
        break;
      }
      case global::states::gameplay: {
        gameplay_input(code, state);
        break;
      }
    }

    if (code == ntf::key_escape && state == ntf::press) {
      ntf::engine_close_window();
    }
  });
}

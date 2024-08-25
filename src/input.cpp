#include "global.hpp"
#include "input.hpp"

#include "ui/frontend.hpp"

#define PRESSED(key) code == key && state == input::keystate::press

static void frontend_input(input::keycode code, input::keystate state) {
  auto& entry = frontend::instance().entry();
  if (PRESSED(input::keycode::key_s)) {
    entry.set_next_index();
  } else if (PRESSED(input::keycode::key_w)) {
    entry.set_prev_index();
  }
  if (PRESSED(input::keycode::key_j)) {
    entry.on_click();
  } else if (PRESSED(input::keycode::key_k)) {
    frontend::instance().pop();
  }
}

static void gameplay_input(input::keycode code, input::keystate state) {
  if (PRESSED(input::keycode::key_i)) {
    global::go_back();
  }
}

static ntf::glfw::window<renderer>* win; // ugly

void input::init(ntf::glfw::window<renderer>& window) {
  win = &window;
  window.set_key_event([&](input::keycode code, auto, input::keystate state, auto) {
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

    if (code == input::keycode::key_escape && state == input::keystate::press) {
      window.close();
    }
  });
}

bool input::poll_key(keycode code, keystate state) {
  return win->poll_key(code, state);
}

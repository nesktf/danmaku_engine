#include <shogle/shogle.hpp>

using namespace ntf::shogle;

int main() {
  ntf::log::set_level(ntf::loglevel::LOG_VERBOSE);
  engine eng{800, 600, "test"};

  eng.set_draw_event([&]() {
    gl::clear_viewport(color3{0.2f});
  });
  eng.set_update_event([&](float) {});

  eng.set_key_event([&](glfw::keycode code, auto, glfw::keystate state, auto) {
    if (code == glfw::key_escape && state == glfw::press) {
      eng.window().close();
    }
  });

  eng.start();
}


#include "stage/state.hpp"

#include "render.hpp"
#include "global.hpp"
#include "resources.hpp"
#include "input.hpp"

#include "ui/frontend.hpp"

namespace global {

static global_state _state;

void tick() {
  _state.elapsed_ticks++;

  switch(_state.current_state) {
    case global::states::gameplay: {
      assert(_state.stage && "Stage not initialized");
      _state.stage->tick();
      break;
    }
    case global::states::frontend: {
      frontend::tick();
      break;
    }
    default: break;
  }
}

void render(double dt, double alpha) {
  render::draw(dt, alpha);
}

void start_stage(std::string path) {
  _state.stage = std::make_unique<stage::state>(path);
  _state.current_state = global::states::gameplay;
}

void go_back() {
  _state.current_state = global::states::frontend;
}

global_state& state() { return _state; }

} // namespace global


int main([[maybe_unused]] const int argc, [[maybe_unused]] const char* argv[]) {
  ntf::log::set_level(ntf::loglevel::verbose);

  auto glfw = ntf::glfw::init();
  auto window = ntf::glfw::window<renderer>{1280, 720, "test"};
  auto imgui = ntf::imgui::init(window, ntf::imgui::glfw_gl3_impl{});

  ntf::glfw::set_swap_interval(0); // disable vsync

  // Common init
  render::init(window);
  input::init(window); // after global?
  res::init();

  // Global handler init
  frontend::init();
  global::_state.current_state = global::states::frontend;

  // Post init
  render::post_init(window);

  window.set_viewport_event(render::viewport_event);

  ntf::shogle_main_loop(window, imgui, UPS, global::render, global::tick);

  res::destroy();
  render::destroy();
  ntf::log::debug("[main] byebye!!");
}

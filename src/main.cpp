#include "stage/state.hpp"

#include "render.hpp"
#include "global.hpp"
#include "resources.hpp"
#include "input.hpp"

#include "ui/frontend.hpp"

namespace global {

static global_state _state;

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

  auto glfw = glfw::init();
  glfw::set_swap_interval(0); // disable vsync
  
  glfw::window<renderer> window{1280, 720, "test"};
  auto imgui = imgui::init(window, imgui::glfw_gl3_impl{});


  // Common init
  render::init(window);
  input::init(window); // after global?
  res::init([&](){
    render::post_init(window);

    // global init
    frontend::init();
    global::_state.current_state = global::states::frontend;
  });


  auto render_fun = [&](double dt, double alpha) {
    imgui.start_frame();
    render::draw(window, dt, alpha);
    imgui.end_frame();
  };

  auto tick_fun = [&]() {
    global::_state.elapsed_ticks++;
    res::do_requests();

    switch(global::_state.current_state) {
      case global::states::gameplay: {
        assert(global::_state.stage && "Stage not initialized");
        global::_state.stage->tick();
        break;
      }
      case global::states::frontend: {
        frontend::tick();
        break;
      }
      default: break;
    }
  };

  ntf::shogle_main_loop(window, UPS, render_fun, tick_fun);

  res::destroy();
  render::destroy();
  ntf::log::debug("[main] byebye!!");
}

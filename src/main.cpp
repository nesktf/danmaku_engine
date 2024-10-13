
#include "render.hpp"
#include "global.hpp"
#include "resources.hpp"
#include "input.hpp"

#include "ui/frontend.hpp"

#include "stage/stage.hpp"

namespace global {

static global_state _state;

void start_stage(std::string path) {
  stage::load(path);
  _state.current_state = global::states::gameplay;
}

void go_back() {
  _state.current_state = global::states::frontend;
}

global_state& state() { return _state; }

} // namespace global

namespace {

using window_type = glfw::window<renderer>;
using imgui_type = imgui::imgui_lib<imgui::glfw_gl3_impl>;

void render_frame(imgui_type& imgui, double dt, double alpha) {
  imgui.start_frame();

  render::clear_viewport();
  switch (global::_state.current_state) {
    case global::states::frontend: {
      render::draw_frontend(dt);
      break;
    }
    case global::states::gameplay: {
      stage::render(dt, alpha);
      break;
    }
    default: break;
  }

  imgui.end_frame();
}

void logic_frame() {
  global::_state.elapsed_ticks++;
  res::do_requests();

  switch(global::_state.current_state) {
    case global::states::gameplay: {
      stage::tick();
      break;
    }
    case global::states::frontend: {
      frontend::tick();
      break;
    }
    default: break;
  }
}

} // namespace

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
    stage::init();
    frontend::init();
    global::_state.current_state = global::states::frontend;
  });

  ntf::shogle_main_loop(window, UPS,
    [&](double dt, double alpha) { render_frame(imgui, dt, alpha); },
    [&]() { logic_frame(); }
  );

  stage::destroy();
  res::destroy();
  render::destroy();
  ntf::log::debug("[main] byebye!!");
}

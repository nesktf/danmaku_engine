#include "render.hpp"
#include "global.hpp"
#include "resources.hpp"
#include "input.hpp"

#include "ui/frontend.hpp"

#include "stage/stage.hpp"


static std::unique_ptr<stage::context> stage_ctx;

namespace global {

static global_state _state;

void start_stage(std::string path) {
  stage_ctx = std::make_unique<stage::context>(path);
  _state.current_state = global::states::gameplay;
}

void go_back() {
  _state.current_state = global::states::frontend;
}

global_state& state() { return _state; }

} // namespace global


int main([[maybe_unused]] const int argc, [[maybe_unused]] const char* argv[]) {
  ntf::log::set_level(ntf::log::level::verbose);

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

  ntf::shogle_main_loop(window, UPS,
    [&](double dt, double alpha) {
      auto& state = global::state();
      imgui.start_frame();

      render::clear_viewport();
      switch (state.current_state) {
        case global::states::frontend: {
          render::draw_frontend(dt);
          break;
        }
        case global::states::gameplay: {
          stage_ctx->render(dt, alpha);
          break;
        }
        default: break;
      }

      imgui.end_frame();
    },
    [&]() {
      auto& state = global::state();
      state.elapsed_ticks++;
      res::do_requests();

      switch(state.current_state) {
        case global::states::gameplay: {
          stage_ctx->tick();
          break;
        }
        case global::states::frontend: {
          frontend::tick();
          break;
        }
        default: break;
      }
    }
  );

  stage_ctx.reset();
  res::destroy();
  render::destroy();
  ntf::log::debug("[main] byebye!!");
}

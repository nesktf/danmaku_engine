
#include "render.hpp"
#include "global.hpp"
#include "resources.hpp"
#include "input.hpp"

#include "ui/frontend.hpp"

#include "game/context.hpp"

namespace {

} // namespace

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

res::manager init_resources() {
  res::manager resources;
  resources.load_defaults();

  resources.request_shader("sprite",
                           "res/shader/sprite.vs.glsl", "res/shader/sprite.fs.glsl");
  resources.request_shader("font",
                           "res/shader/font.vs.glsl", "res/shader/font.fs.glsl");
  resources.request_shader("framebuffer",
                           "res/shader/framebuffer.vs.glsl", "res/shader/framebuffer.fs.glsl");
  resources.request_shader("frontend",
                           "res/shader/frontend.vs.glsl", "res/shader/frontend.fs.glsl");

  resources.request_font("arial",
                         "res/fonts/arial.ttf");
  
  return resources;
}

} // namespace


int main([[maybe_unused]] const int argc, [[maybe_unused]] const char* argv[]) {
#ifdef NDEBUG
  logger::set_level(logger::level::info);
#else
  logger::set_level(logger::level::verbose);
#endif

  auto glfw = glfw::init();
  glfw::set_swap_interval(0); // disable vsync
  
  glfw::window<renderer> window{1280, 720, "test"};
  auto imgui = imgui::init(window, imgui::glfw_gl3_impl{});

  // Common init
  render::init(window);
  input::init(window); // after global?

  {
    ntf::async_data_loader async_loader;
    res::manager resources = init_resources();
    std::unique_ptr<game::context> context;
    render::stage_viewport vp;

    resources.init_requests(async_loader, [&]() {
      render::post_init(window, resources);

      vp.init(VIEWPORT, VIEWPORT/2, (vec2)VIEWPORT*.5f, resources.shader_at("framebuffer"));
      frontend::init();
      global::_state.current_state = global::states::frontend;
    });


    ntf::shogle_main_loop(window, UPS,
      [&](double dt, double alpha) { render_frame(imgui, dt, alpha); },
      [&]() { async_loader.do_requests(); logic_frame(); }
    );

    vp.destroy();
  }

  // stage::destroy();
  // res::destroy();
  render::destroy();
  logger::debug("[main] byebye!!");
}

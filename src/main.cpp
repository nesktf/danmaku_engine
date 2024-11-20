#include "context.hpp"

namespace okuu {

okuu::context* global_ctx;

context::context(window_type* window) : _window(window) {
  render::init(*_window);
  _window->set_key_event([this](keycode code, auto, keystate state, auto) {
    _handle_input(code, state);
  });

  _resources.load_defaults();

  _resources.request_shader("sprite",
                            "res/shader/sprite.vs.glsl", "res/shader/sprite.fs.glsl");
  _resources.request_shader("font",
                            "res/shader/font.vs.glsl", "res/shader/font.fs.glsl");
  _resources.request_shader("framebuffer",
                            "res/shader/framebuffer.vs.glsl", "res/shader/framebuffer.fs.glsl");
  _resources.request_shader("frontend",
                            "res/shader/frontend.vs.glsl", "res/shader/frontend.fs.glsl");

  _resources.request_font("arial",
                          "res/fonts/arial.ttf");

  _resources.init_requests(_loader, [this]() {
    render::post_init(*_window);
    frontend::init();
    _curr_scr = okuu::screen::frontend;
  });
  logger::debug("[okuu::context] Initialized");
}

context::~context() noexcept {
  render::destroy();
  logger::debug("[okuu:context] byebye!!");
}

void context::render(double dt, double alpha) {
  render::clear_viewport();
  switch(_curr_scr) {
    case okuu::screen::frontend: {
      render::draw_frontend(dt);
      break;
    }
    case okuu::screen::gameplay: {
      _stage->render(dt, alpha);
      break;
    }
    default: break;
  }
  _elapsed_frames++;
}

void context::tick() {
  _loader.do_requests();
  switch (_curr_scr) {
    case okuu::screen::frontend: {
      frontend::tick();
      break;
    }
    case okuu::screen::gameplay: {
      _stage->tick();
      break;
    }
    default: break;
  }
  _elapsed_ticks++;
}

void context::start_stage(std::string path) {
  _stage = std::make_unique<stage::context>(path);
  _curr_scr = okuu::screen::gameplay;
}

void context::unload_stage() {
  _curr_scr = okuu::screen::frontend;
}

bool context::poll_key(keycode code, keystate state) const {
  return _window->poll_key(code, state);
}

context& ctx() { return *global_ctx; }

} // namespace okuu

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

  okuu::context ctx{&window};
  okuu::global_ctx = &ctx;

  ntf::shogle_main_loop(window, UPS,
    [&](double dt, double alpha) {
      imgui.start_frame();
      ctx.render(dt, alpha);
      imgui.end_frame();
    },
    [&]() { ctx.tick(); }
  );
}

#include "context.hpp"

namespace okuu {

context::context(okuu::window& win) : _window(&win) {
  _global_ctx = this;
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

} // namespace okuu

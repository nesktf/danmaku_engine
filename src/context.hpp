#pragma once

#include "okuu.hpp"
#include "resources.hpp"
// #include "stage/stage.hpp"

// #include "ui/frontend.hpp"

namespace okuu {

enum class screen {
  loading = 0,
  frontend,
  gameplay
};

using viewport_event = ntf::event<std::size_t, std::size_t>;
using input_event = ntf::event<keycode, keystate>;

class context {
public:
  context(okuu::window& win);

public:
  void on_render(double dt, double alpha);
  void on_fixed_update();

  void start_stage(std::string path);

  void unload_stage();

  viewport_event::subscription vp_subscribe(std::function<void(std::size_t, std::size_t)> cb);
  input_event::subscription vp_subscribe(std::function<void(keycode, keystate)> cb);

  bool poll_key(okuu::keycode code, okuu::keystate state = okuu::keystate::press) const {
    return _window->poll_key(code, state);
  }

public:
  okuu::resource_manager& res() { return _resources; }
  const okuu::resource_manager& res() const { return _resources; }

private:
  void _handle_input(keycode code, keystate state);

private:
  ntf::async_data_loader _loader;
  okuu::resource_manager _resources;
  // std::unique_ptr<stage::context> _stage;
  okuu::window* _window;
  okuu::screen _curr_scr{okuu::screen::loading};

  okuu::frames _elapsed_ticks{0};
  okuu::frames _elapsed_frames{0};

  okuu::viewport_event _vp_event;
  okuu::input_event _input_event;

public:
  static context& get() {
    NTF_ASSERT(_global_ctx, "Context not initialized");
    return *_global_ctx;
  }

private:
  static inline context* _global_ctx {nullptr};

public:
  NTF_DECLARE_NO_MOVE_NO_COPY(context);
};

} // namespace okuu

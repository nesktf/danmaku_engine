#pragma once

#include "stage/stage.hpp"

#include "ui/frontend.hpp"

namespace okuu {

enum class screen {
  loading = 0,
  frontend,
  gameplay
};

using viewport_event = ntf::event<std::size_t, std::size_t>;
using input_event = ntf::event<keycode, keystate>;
using arena = ntf::memory_arena<4096>;

class context {
public:
  context(window_type* window);

public:
  void render(double dt, double alpha);
  void tick();

  void start_stage(std::string path);

  void unload_stage();

  viewport_event::subscription vp_subscribe(std::function<void(std::size_t, std::size_t)> cb);
  input_event::subscription vp_subscribe(std::function<void(keycode, keystate)> cb);

  bool poll_key(keycode code, keystate state = keystate::press) const;

public:
  res::manager& res() { return _resources; }
  const res::manager& res() const { return _resources; }

private:
  void _handle_input(keycode code, keystate state);

private:
  ntf::async_data_loader _loader;
  res::manager _resources;
  std::unique_ptr<stage::context> _stage;
  window_type* _window;
  okuu::screen _curr_scr{okuu::screen::loading};

  frames _elapsed_ticks{0};
  frames _elapsed_frames{0};

  viewport_event _vp_event;
  input_event _input_event;

  arena _arena;

public:
  NTF_DECLARE_NO_MOVE_NO_COPY(context);
};

context& ctx();

} // namespace okuu

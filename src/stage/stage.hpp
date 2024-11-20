#pragma once

#include "stage/projectile.hpp"
#include "stage/player.hpp"
#include "stage/boss.hpp"

#include "render.hpp"

namespace okuu::stage {

class context {
private:
  enum class state {
    loading = 0,
    main,
    end,
  };

public:
  context(std::string_view script_path);

public:
  void tick();
  void render(double dt, [[maybe_unused]] double alpha);

public:
  void start();

private:
  void _lua_post_open(sol::table stlib);
  void _prepare_player();

private:
  context::state _state{context::state::loading};

  sol::state _lua;

  sol::coroutine _task;
  sol::thread _task_thread;

  sol::protected_function _on_tick;
  sol::protected_function _on_render;

  frames _tick_count{0}, _frame_count{0};
  frames _task_time{0}, _task_wait{0};

  std::list<stage::projectile> _projs;
  std::optional<stage::boss> _boss;
  std::optional<stage::player> _player;

public:
  ~context() noexcept;
  NTF_DISABLE_COPY(context);
};

} // namespace okuu::stage

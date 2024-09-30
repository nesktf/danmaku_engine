#pragma once

#include <sol/sol.hpp>

#include "global.hpp"
#include "render.hpp"
#include "resources.hpp"

#include "game/entity.hpp"

#include <shogle/core/allocator.hpp>

#include <list>

namespace game {

class context {
private:
  enum class ctx_state : uint8_t {
    preload = 0,
    loading,
    main,
  };

public:
  struct context_args {
    std::string script;
    ivec2 viewport;
  };

public:
  context(context_args args);

public:
  bool init(ntf::async_data_loader& loader);

  void tick();
  void draw(double dt, double alpha, render::stage_viewport& vp);

public:
  const context_args& args() const { return _args; }

private:
  void _prepare_player();

  void _prepare_lua_env();
  void _open_lua_libs();

  void _update_stage();
  void _draw_stage([[maybe_unused]] double dt, [[maybe_unused]] double alpha,
                   render::stage_viewport& vp);

private:
  context_args _args;
  sol::state _lua;
  sol::protected_function _entrypoint;

  frames _tick_count{0}, _frame_count{0};
  frames _task_time{0}, _task_wait{0};

  std::list<entity::projectile> _projectiles;
  entity::boss _boss;
  entity::player _player;

  res::manager _res;

  ctx_state _state{ctx_state::preload};
};

} // namespace game

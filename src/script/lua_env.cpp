#define SOL_ALL_SAFETIES_ON 1
#include "script/lua_env.hpp"

#include "stage/state.hpp"

#include <shogle/core/error.hpp>
#include <shogle/core/log.hpp>

namespace script {

lua_env::lua_env(stage::state* stage, std::string_view stage_script) {
  _lua.open_libraries(
    sol::lib::base, sol::lib::coroutine,
    sol::lib::package, sol::lib::table,
    sol::lib::math, sol::lib::string
  );
  _populate_globals(stage);

  _lua.script_file(stage_script.data());

  sol::protected_function init_task = _lua["__INIT_TASK"];
  if (init_task) {
    init_task();
  } else {
    ntf::log::warning("[stage::lua_env] Lua initial task not defined in script {}", stage_script);
  }

  _main_task = _lua["__MAIN_TASK"];
  if (!_main_task) {
    throw ntf::error{"[stage::lua_env] Lua main task not defined in script {}", stage_script};
  }
}

void lua_env::_populate_globals(stage::state* stage) {
  _lua["__GLOBAL"] = _lua.create_table_with(
    "dt", DT,
    "ups", UPS,
    "ticks", 0
  );
  _lua["package"]["path"] = ";res/script/?.lua";

  // Log functions
  _lua.set_function("__LOG_ERROR", [](std::string msg) { ntf::log::error("{}", msg); });
  _lua.set_function("__LOG_WARNING", [](std::string msg) { ntf::log::warning("{}", msg); });
  _lua.set_function("__LOG_DEBUG", [](std::string msg) { ntf::log::debug("{}", msg); });
  _lua.set_function("__LOG_INFO", [](std::string msg) { ntf::log::info("{}", msg); });
  _lua.set_function("__LOG_VERBOSE", [](std::string msg) { ntf::log::verbose("{}", msg); });

  // Boss functions
  _lua.set_function("__SPAWN_BOSS", [stage](float scale, float ang_speed, sol::table p0, sol::table p1) {
    auto atlas_id = res::atlas_from_name("effects");
    auto group_handle = atlas_id->get().find_group("stars_small");
    const auto index = atlas_id->get().group_at(group_handle.value())[0];

    stage->boss.set_sprite(res::sprite{
      .handle = atlas_id.value(),
      .index = index,
    });

    stage->boss.set_scale(scale);
    stage->boss.set_angular_speed(ang_speed);

    cmplx first = cmplx{p0.get<float>("real"), p0.get<float>("imag")};
    cmplx last = cmplx{p1.get<float>("real"), p1.get<float>("imag")};

    stage->boss.init(first, 
      entity::move_towards(last, DT*cmplx{10.0f,10.0f}, cmplx{DT}, 0.8)
    );
  });
  _lua.set_function("__MOVE_BOSS", [stage](sol::table pos) {
    cmplx new_pos = cmplx{pos.get<float>("real"), pos.get<float>("imag")};
    stage->boss.set_movement(entity::move_towards(new_pos, DT*cmplx{10.0f,10.0f}, cmplx{DT}, .8f));
  });
  _lua.set_function("__BOSS_POS", [this, stage]() -> sol::table {
    const auto pos = stage->boss.transform().pos();
    return _lua.create_table_with("real", pos.x, "imag", pos.y);
  });
  
  // Player functions
  _lua.set_function("__PLAYER_POS", [this, stage]() -> sol::table {
    const auto pos = stage->player.transform().pos();
    return _lua.create_table_with("x", pos.x, "y", pos.y);
  });

  // Projectile functions
  _lua.set_function("__SPAWN_DANMAKU", [stage](sol::table dir, float speed, sol::table pos) {
    cmplx proj_dir = cmplx{dir.get<float>("real"), dir.get<float>("imag")};
    cmplx proj_pos = cmplx{pos.get<float>("real"), pos.get<float>("imag")};

    auto atlas_id = res::atlas_from_name("effects");
    // auto group_handle = atlas_id->get().find_group("stars_small");
    // const auto index = atlas_id->get().group_at(group_handle.value())[0];
    stage->projectiles.emplace_back(res::sprite{
        .handle = atlas_id.value(),
        .sequence = atlas_id->get().find_sequence("stars_small.test"),
      }, 
      entity::move_linear(proj_dir*speed), proj_pos, stage->ticks());
  });

  // Misc
  _lua.set_function("__STAGE_TICKS", [stage]() -> frames {
    return stage->ticks();
  });
  _lua.set_function("__VIEWPORT", [this]() -> sol::table {
    return _lua.create_table_with("x", VIEWPORT.x, "y", VIEWPORT.y);
  });
}

frame_delay lua_env::call_task() {
  assert(_main_task && "Loaded stage main task is invalid!!");
  sol::table task_yield = _main_task();
  const auto delay = task_yield.get<frame_delay>("wait_time");
  return delay;
}

} // namespace script

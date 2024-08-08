#define SOL_ALL_SAFETIES_ON 1

#include "global.hpp"
// #include "stage.hpp"
#include "resources.hpp"
#include "entity/boss.hpp"

#include <shogle/core/log.hpp>

namespace stage {

void stage_state::load_env(std::string_view stage_script) {
  // Reset previous state (if any)
  if (_frames > 0) {
    _frames = 0;
    _main_task.reset();
    _lua = sol::state{};
    _task_time = 0;
    _task_wait = 0;
  }

  // Load new environment
  _prepare_lua_env();
  _lua.script_file(stage_script.data());

  sol::protected_function init_task = _lua["__INIT_TASK"];
  if (init_task) {
    init_task();
  } else {
    ntf::log::warning("Lua init not defined");
  }

  _main_task = _lua["__MAIN_TASK"];
  if (!_main_task) {
    ntf::log::error("Lua main task not defined!!!");
  }

  player.set_pos((vec2)VIEWPORT*0.5f);
  player.set_scale(40.0f);
  player.set_sprite(res::spritesheet("effects").sprite_at("stars_small", 0));
}

void stage_state::tick() {
  ++_frames;

  if (_main_task && _task_time >= _task_wait) {
    _task_time = 0;
    _lua["__GLOBAL"]["ticks"] = _frames;

    sol::table wait_info = _main_task();
    
    int time = wait_info.get<int>("wait_time");
    if (time < 0) {
      _main_task.reset();
      ntf::log::debug("Lua main task returned");
    } else {
      _task_wait = time;
    }
  }
  _task_time++;

  for (auto& projectile : projectiles) {
    projectile.sprite = res::spritesheet("effects").sprite_at("stars_small", "test", _frames+projectile.birth);
    projectile.tick();
  }

  if (boss.ready()) {
    boss.tick();
  }
  player.tick();

  _clean_oob();
}

void stage_state::_prepare_lua_env() {
  _lua.open_libraries(
    sol::lib::base, sol::lib::coroutine, 
    sol::lib::math, sol::lib::table, 
    sol::lib::package, sol::lib::string
  );

  sol::table vp = _lua.create_table_with("x", VIEWPORT.x, "y", VIEWPORT.y);
  _lua["__GLOBAL"] = _lua.create_table_with(
    "dt", DT,
    "ups", UPS,
    "ticks", 0,
    "viewport", vp
  );

  _lua["package"]["path"] = ";res/script/?.lua";

  _lua.set_function("__LOG_ERROR", [](std::string msg) { ntf::log::error("{}", msg); });
  _lua.set_function("__LOG_WARNING", [](std::string msg) { ntf::log::warning("{}", msg); });
  _lua.set_function("__LOG_DEBUG", [](std::string msg) { ntf::log::debug("{}", msg); });
  _lua.set_function("__LOG_INFO", [](std::string msg) { ntf::log::info("{}", msg); });
  _lua.set_function("__LOG_VERBOSE", [](std::string msg) { ntf::log::verbose("{}", msg); });
  _lua.set_function("__SPAWN_BOSS", [this](float scale, float ang_speed, sol::table p0, sol::table p1) {
    boss.set_sprite(res::spritesheet("effects").sprite_at("stars_small", 0));

    boss.set_scale(scale);
    boss.set_angular_speed(ang_speed);

    cmplx first = cmplx{p0.get<float>("real"), p0.get<float>("imag")};
    cmplx last = cmplx{p1.get<float>("real"), p1.get<float>("imag")};

    boss.init(first, 
      entity::move_towards(last, DT*cmplx{10.0f,10.0f}, cmplx{DT}, 0.8)
    );
  });
  _lua.set_function("__MOVE_BOSS", [this](sol::table pos) {
    cmplx new_pos = cmplx{pos.get<float>("real"), pos.get<float>("imag")};
    boss.set_movement(entity::move_towards(new_pos, DT*cmplx{10.0f,10.0f}, cmplx{DT}, .8f));
  });
  _lua.set_function("__PLAYER_POS", [this]() -> sol::table {
    const auto pos = player.transform().pos();
    return _lua.create_table_with("x", pos.x, "y", pos.y);
  });
  _lua.set_function("__BOSS_POS", [this]() -> sol::table {
    const auto pos = boss.transform().pos();
    return _lua.create_table_with("real", pos.x, "imag", pos.y);
  });
  _lua.set_function("__SPAWN_DANMAKU", [this](sol::table dir, float speed, sol::table pos) {
    cmplx proj_dir = cmplx{dir.get<float>("real"), dir.get<float>("imag")};
    cmplx proj_pos = cmplx{pos.get<float>("real"), pos.get<float>("imag")};
    sprite star = res::spritesheet("effects").sprite_at("stars_small", 0);
    projectiles.emplace_back(star, entity::move_linear(proj_dir*speed), proj_pos, _frames);
  });
}

void stage_state::_clean_oob() {
  const float extra = 10.f;
  std::erase_if(projectiles, [&](const auto& projectile) { 
    const auto pos = projectile.transform.pos();
    return (pos.x < -extra || pos.y < -extra || pos.x > VIEWPORT.x+extra || pos.y > VIEWPORT.y+extra);
  });
}

} // namespace stage

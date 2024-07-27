#include "stage.hpp"

#define SOL_ALL_SAFETIES_ON 1
#include <sol/sol.hpp>

#include "global.hpp"
#include "resources.hpp"

#include <shogle/core/log.hpp>
#include "entity/projectile.hpp"

using namespace ntf;

static struct {
  sol::state lua_state;
  sol::protected_function level_task;
} stage;

static const char* script_path = "script/test_level.lua";

void ntf::stage_init() {
  auto& lua = stage.lua_state;
  lua.open_libraries(sol::lib::base, sol::lib::coroutine, sol::lib::math, sol::lib::table);

  lua.set_function("log_debug", [](std::string message) {
    log::debug("[stage::lua] {}", message);
  });

  lua.set_function("create_bullet", [&](float vel_x, float vel_y, float acc_x, float acc_y, float ret) {
    const auto& sprites = res.sprites;
    const shogle::sprite* bullet = &sprites.at("effects").at("stars_small");

    entity_movement movement {
      .velocity = cmplx{vel_x, vel_y},
      .acceleration = cmplx{acc_x, acc_y},
      .retention = ret,
    };

    global.projectiles.emplace_back(projectile{bullet, movement});
  });

  lua.script_file(script_path);

  sol::protected_function entrypoint = lua["INIT"];
  if (entrypoint) {
    entrypoint();
  } else {
    log::error("Lua entrypoint failed");
  }
  stage.level_task = lua["NEXT_TASK"];
  if (!stage.level_task) {
    log::error("Lua next task get failed");
  }
}

static uint wait_time {0}, time_counter {0};

void ntf::stage_next_tick() {
  if (time_counter >= wait_time) {
    time_counter = 0;
    wait_time = stage.level_task();
  }
  time_counter++;
  for (auto& bullet : global.projectiles) {
    bullet.tick();
  }
}

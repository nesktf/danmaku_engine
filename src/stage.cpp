#define SOL_ALL_SAFETIES_ON 1
#include <sol/sol.hpp>

#include "stage.hpp"
#include "resources.hpp"

#include <shogle/core/log.hpp>

static struct {
  vector<entity::projectile> projectiles;
  entity::player player;
  sol::state lua;
  sol::protected_function level_task;
  uint frames;
} state;

vector<entity::projectile>& stage::projectiles() {
  return state.projectiles;
}

entity::player& stage::player() {
  return state.player;
}

static const char* script_path = "script/test_level.lua";

void stage::init() {
  auto& lua = state.lua;
  lua.open_libraries(sol::lib::base, sol::lib::coroutine, sol::lib::math, sol::lib::table);

  lua.set_function("log_debug", [](std::string message) {
    ntf::log::debug("[stage::lua] {}", message);
  });

  lua.set_function("create_bullet", [&](float vel_x, float vel_y, float acc_x, float acc_y, float ret) {
    const auto sprite = res::spritesheet("effects").sprite_at("stars_small", 0);

    entity::movement movement {
      .velocity = cmplx{vel_x, vel_y},
      .acceleration = cmplx{acc_x, acc_y},
      .retention = ret,
    };

    entity::projectile proj{sprite, movement, state.frames};
    proj.transform.set_pos((vec2)VIEWPORT*0.5f);

    state.projectiles.emplace_back(std::move(proj));
  });

  lua.script_file(script_path);

  sol::protected_function entrypoint = lua["INIT"];
  if (entrypoint) {
    entrypoint();
  } else {
    ntf::log::error("Lua entrypoint failed");
  }
  state.level_task = lua["NEXT_TASK"];
  if (!state.level_task) {
    ntf::log::error("Lua next task get failed");
  }
}

static bool is_oob(const vec2& pos) {
  const auto extra = 10.f;
  return (pos.x < -extra || pos.y < -extra || pos.x > VIEWPORT.x+extra || pos.y > VIEWPORT.y+extra);
}

static void clean_garbage() {
  std::erase_if(state.projectiles, [](const auto& proj) { return is_oob(proj.transform.pos()); });
}

static uint wait_time {0}, time_counter {0};

void stage::tick() {
  state.frames++;
  if (time_counter >= wait_time) {
    time_counter = 0;
    wait_time = state.level_task();
  }
  time_counter++;

  for (auto& bullet : state.projectiles) {
    bullet.sprite = res::spritesheet("effects").sprite_at("stars_small", "test", state.frames+bullet.birth);
    bullet.tick();
  }

  state.player.tick();

  clean_garbage();
}

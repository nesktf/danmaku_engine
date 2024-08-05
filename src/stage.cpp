#define SOL_ALL_SAFETIES_ON 1
#include <sol/sol.hpp>

#include "stage.hpp"
#include "resources.hpp"
#include "entity/boss.hpp"

#include <shogle/core/log.hpp>

static struct {
  vector<entity::projectile> projectiles;
  entity::player player;
  entity::boss boss;
  sol::state lua;
  sol::protected_function level_task;
  bool call_thing = true;
  uint frames = 0;
} state;

vector<entity::projectile>& stage::projectiles() {
  return state.projectiles;
}

entity::player& stage::player() {
  return state.player;
}

entity::boss& stage::boss() {
  return state.boss;
}

static const char* script_path = "res/script/stage/test.lua";

void stage::init() {
  auto& lua = state.lua;
  lua.open_libraries(
    sol::lib::base, sol::lib::coroutine, sol::lib::math, sol::lib::table, sol::lib::package, sol::lib::string);

  lua["__GLOBAL"] = lua.create_table_with(
    "dt", DT,
    "ups", UPS,
    "ticks", state.frames
  );

  lua["package"]["path"] = ";/mnt/patchouli/projects/dev/local/shogle_tests/res/script/?.lua";

  lua.set_function("__LOG_ERROR", [](std::string msg) { ntf::log::error("{}", msg); });
  lua.set_function("__LOG_WARNING", [](std::string msg) { ntf::log::warning("{}", msg); });
  lua.set_function("__LOG_DEBUG", [](std::string msg) { ntf::log::debug("{}", msg); });
  lua.set_function("__LOG_INFO", [](std::string msg) { ntf::log::info("{}", msg); });
  lua.set_function("__LOG_VERBOSE", [](std::string msg) { ntf::log::verbose("{}", msg); });

  // lua.set_function("create_bullet", [&](float vel_x, float vel_y, float acc_x, float acc_y, float ret) {
  //   const auto sprite = res::spritesheet("effects").sprite_at("stars_small", 0);
  //
  //   entity::movement movement {
  //     .velocity = cmplx{vel_x, vel_y},
  //     .acceleration = cmplx{acc_x, acc_y},
  //     .retention = ret,
  //   };
  //
  //   entity::projectile proj{sprite, movement, state.frames};
  //   proj.transform.set_pos((vec2)VIEWPORT*0.5f);
  //
  //   state.projectiles.emplace_back(std::move(proj));
  // });

  lua.script_file(script_path);

  sol::protected_function init_task = lua["__INIT_TASK"];
  if (init_task) {
    init_task();
  } else {
    ntf::log::warning("Lua init not defined");
  }

  state.level_task = lua["__MAIN_TASK"];
  if (!state.level_task) {
    ntf::log::error("Lua main task not defined!!!");
  }

  state.player.set_pos((vec2)VIEWPORT*0.5f);
  state.player.set_scale(40.0f);
  state.player.set_sprite(res::spritesheet("effects").sprite_at("stars_small", 0));

  state.boss.set_scale(50.0f);
  state.boss.set_angular_speed(PI);
  state.boss.set_sprite(res::spritesheet("effects").sprite_at("stars_small", 0));
  state.boss.init(vec2{-10.0f, -10.0f}, 
    entity::move_towards(cmplx{VIEWPORT.x*0.5f, VIEWPORT.y*0.25f}, DT*cmplx{10.0f,10.0f}, cmplx{DT}, 0.8)
  );
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
  if (state.call_thing && time_counter >= wait_time) {
    time_counter = 0;
    state.lua["__GLOBAL"]["ticks"] = state.frames;
    sol::table wait_info = state.level_task();
    
    int time = wait_info.get<int>("wait_time");
    if (time < 0) {
      state.call_thing = false;
      ntf::log::debug("Lua main task returned");
    } else {
      wait_time = time;
    }
  }
  time_counter++;

  for (auto& bullet : state.projectiles) {
    bullet.sprite = res::spritesheet("effects").sprite_at("stars_small", "test", state.frames+bullet.birth);
    bullet.tick();
  }

  if (state.boss.ready()) {
    state.boss.tick();
  }
  state.player.tick();

  clean_garbage();
}

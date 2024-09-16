#define SOL_ALL_SAFETIES_ON 1

#include "stage/state.hpp"

#include "resources.hpp"

namespace stage {

state::state(std::string_view stage_script) {
  _lua.open_libraries(
    sol::lib::base, sol::lib::coroutine,
    sol::lib::package, sol::lib::table,
    sol::lib::math, sol::lib::string
  );
  _setup_lua_env();

  _lua.script_file(stage_script.data());

  sol::protected_function init_task = _lua["__INIT_TASK"];
  if (init_task) {
    init_task();
  } else {
    ntf::log::warning("[stage::lua_env] Lua initial task not defined in script {}", stage_script);
  }

  _entrypoint = _lua["__MAIN_TASK"];
  if (!_entrypoint) {
    throw ntf::error{"[stage::lua_env] Lua main task not defined in script {}", stage_script};
  }

  _prepare_player();
}

void state::_setup_lua_env() {
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
  _lua.set_function("__SPAWN_BOSS", [this](float scale, float, sol::table p0, sol::table p1) {
    const auto atlas = res::get_atlas("effects").value();
    const auto entry = atlas->group_at(atlas->find_group("ball_solid").value())[0];
    const auto aspect = atlas->at(entry).aspect();

    cmplx first = cmplx{p0.get<float>("real"), p0.get<float>("imag")};
    cmplx last = cmplx{p1.get<float>("real"), p1.get<float>("imag")};

    ntf::transform2d transform;
    transform
      .set_pos(first)
      .set_scale(scale*aspect);

    boss = boss_entity{boss_entity::args{
      .transform = transform,
      .movement = entity_movement_towards(last, DT*cmplx{10.f,10.f}, cmplx{DT}, .8f),
      .animator = entity_animator_static(atlas, entry),
    }};
  });

  _lua.set_function("__MOVE_BOSS", [this](sol::table pos) {
    cmplx new_pos = cmplx{pos.get<float>("real"), pos.get<float>("imag")};
    boss.movement = entity_movement_towards(new_pos, DT*cmplx{10.f,10.f}, cmplx{DT}, .8f);
  });

  _lua.set_function("__BOSS_POS", [this]() -> sol::table {
    const auto pos = boss.transform.pos();
    return _lua.create_table_with("real", pos.x, "imag", pos.y);
  });
  
  // Player functions
  _lua.set_function("__PLAYER_POS", [this]() -> sol::table {
    const auto pos = player.transform.pos();
    return _lua.create_table_with("x", pos.x, "y", pos.y);
  });

  // Projectile functions
  _lua.set_function("__SPAWN_DANMAKU", [this](sol::table dir, float speed, sol::table pos) {
    cmplx proj_dir = cmplx{dir.get<float>("real"), dir.get<float>("imag")};
    cmplx proj_pos = cmplx{pos.get<float>("real"), pos.get<float>("imag")};

    const auto atlas = res::get_atlas("effects").value();
    const auto entry = atlas->group_at(atlas->find_group("star_med").value())[1];
    const auto aspect = atlas->at(entry).aspect();

    ntf::transform2d transform;
    transform
      .set_pos(proj_pos)
      .set_scale(aspect*20.f);

    projectiles.emplace_back(projectile_entity::args {
      .transform = transform,
      .movement = entity_movement_linear(proj_dir*speed),
      .animator = entity_animator_static(atlas, entry),
      .angular_speed = 2*M_PIf*DT,
    });
  });

  // Misc
  _lua.set_function("__STAGE_TICKS", [this]() -> frames {
    return ticks();
  });
  _lua.set_function("__VIEWPORT", [this]() -> sol::table {
    return _lua.create_table_with("x", VIEWPORT.x, "y", VIEWPORT.y);
  });
}

void state::_prepare_player() {
  const auto atlas = res::get_atlas("chara_cirno").value();
  const player_entity::animator_type::animation_data anim {
    atlas->find_sequence("cirno.idle").value(),

    atlas->find_sequence("cirno.left").value(),
    atlas->find_sequence("cirno.left_to_idle").value(),
    atlas->find_sequence("cirno.idle_to_left").value(),

    atlas->find_sequence("cirno.right").value(),
    atlas->find_sequence("cirno.right_to_idle").value(),
    atlas->find_sequence("cirno.idle_to_right").value(),
  };
  const vec2 sprite_aspect = atlas->at(atlas->sequence_at(anim[0])[0]).aspect();

  ntf::transform2d transform;
  transform
    .set_pos((vec2)VIEWPORT*.5f)
    .set_scale(sprite_aspect*70.f);

  player = player_entity{player_entity::args{
    .transform = transform,
    .atlas = atlas,
    .anim = anim,
    .base_speed = 350.f*DT,
    .slow_speed = 350.f*.66f*DT,
  }};
}

void state::state::tick() {
  ++_tick_count;

  if (_task_time >= _task_wait) {
    _task_time = 0;

    sol::table task_yield = _entrypoint();
    const auto delay = task_yield.get<frame_delay>("wait_time");
    if (delay < 0) {
      ntf::log::debug("[stage::state] Lua main task returned");
    } else {
      _task_wait = delay;
    }
  }
  _task_time++;

  for (auto& projectile : projectiles) {
    projectile.tick();
  }

  if (!boss.hide) {
    boss.tick();
  }

  player.tick();

  _clean_oob();
}

void stage::state::_clean_oob() {
  const float extra = 10.f;
  std::erase_if(projectiles, [&](const auto& projectile) { 
    const auto pos = projectile.transform.pos();
    return (
      pos.x < -extra || 
      pos.y < -extra || 
      pos.x > VIEWPORT.x+extra || 
      pos.y > VIEWPORT.y+extra
    );
  });
}

} // namespace stage

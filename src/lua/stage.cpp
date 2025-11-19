#include "../stage/entity.hpp"
#include "./stage_env.hpp"

namespace okuu::stage {

namespace {

class lua_player {};

fn prep_player_usertype(sol::table& module) {
  // clang-format off
  return module.new_usertype<lua_player>(
    "player", sol::no_constructor
  );
  // clang-format on
}

fn prep_boss_usertype(sol::table& module) {
  // clang-format off
  return module.new_usertype<lua_boss>(
    "boss", sol::no_constructor,
    "get_slot", &lua_boss::get_slot,
    "kill", &lua_boss::kill,
    "is_alive", &lua_boss::is_alive
  );
  // clang-format on
}

fn prep_proj_usertype(sol::table& module) {
  // clang-format off
  return module.new_usertype<lua_proj>(
    "projectile", sol::no_constructor,
    "get_ticks", &lua_proj::get_ticks,
    "is_alive", &lua_proj::is_alive,
    "kill", &lua_proj::kill
  );
  // clang-format on
}

fn prep_stage_usertype(sol::table& module) {
  // clang-format off
  return module.new_usertype<lua_stage>(
    "stage", sol::no_constructor,
    "yield", sol::yielding(&lua_stage::yield),
    "yield_ticks", sol::yielding(&lua_stage::yield_ticks),
    "yield_secs", sol::yielding(&lua_stage::yield_secs),
    "trigger_dialog", &lua_stage::trigger_dialog,
    "get_player", &lua_stage::get_player,
    "get_boss", &lua_stage::get_boss,
    "spawn_proj", &lua_stage::spawn_proj,
    "spawn_proj_n", &lua_stage::spawn_proj_n
  );
  // clang-format on
}

} // namespace

sol::table setup_stage_module(sol::table& okuu_lib, stage::stage_scene& scene) {
  sol::table stage_module = okuu_lib["stage"].get_or_create<sol::table>();
  prep_player_usertype(stage_module);
  prep_boss_usertype(stage_module);
  prep_proj_usertype(stage_module);
  prep_stage_usertype(stage_module);
  return stage_module;
}

} // namespace okuu::stage

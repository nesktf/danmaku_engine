#include "../stage/entity.hpp"
#include "./stage_env.hpp"

#include "../stage/stage.hpp"

namespace okuu::lua {

static lua_stage get_stage(sol::this_state ts) {
  sol::state_view lua{ts};
  auto okuu_lib = lua["okuu"].get<sol::table>();
  return okuu_lib["__curr_stage"].get<lua_stage>();
}

lua_player::lua_player() {}

static fn prep_player_usertype(sol::table& module) {
  // clang-format off
  return module.new_usertype<lua_player>(
    "player", sol::no_constructor
  );
  // clang-format on
}

lua_boss::lua_boss(u32 slot) noexcept : _boss_slot{slot} {}

bool lua_boss::is_alive(sol::this_state ts) const {
  auto stage = get_stage(ts);
  return stage.get_scene().get_boss(_boss_slot).is_active();
}

void lua_boss::kill(sol::this_state ts) {
  auto stage = get_stage(ts);
  stage.get_scene().kill_boss(_boss_slot);
}

static fn prep_boss_usertype(sol::table& module) {
  // clang-format off
  return module.new_usertype<lua_boss>(
    "boss", sol::no_constructor,
    "get_slot", &lua_boss::get_slot,
    "kill", &lua_boss::kill,
    "is_alive", &lua_boss::is_alive
  );
  // clang-format on
}

lua_projectile::lua_projectile(u64 handle) : _handle{handle} {}

bool lua_projectile::is_alive(sol::this_state ts) const {
  auto stage = get_stage(ts);
  return stage.get_scene().is_projectile_alive(_handle);
}

void lua_projectile::kill(sol::this_state ts) {
  auto stage = get_stage(ts);
  stage.get_scene().kill_projectile(_handle);
}

static fn prep_proj_usertype(sol::table& module) {
  // clang-format off
  return module.new_usertype<lua_projectile>(
    "projectile", sol::no_constructor,
    "is_alive", &lua_projectile::is_alive,
    "kill", &lua_projectile::kill
  );
  // clang-format on
}

static constexpr u32 secs_to_ticks(f32 secs) noexcept {
  return static_cast<u32>(std::floor(secs * okuu::GAME_UPS));
}

lua_stage::lua_stage(stage::stage_scene& scene) : _scene{scene} {}

void lua_stage::on_yield(u32 ticks) {
  _scene->task_wait(ticks);
}

lua_player lua_stage::get_player() {
  return {};
}

void lua_stage::trigger_dialog(std::string dialog_id) {
  NTF_UNUSED(dialog_id);
}

sol::variadic_results lua_stage::get_boss(sol::this_state ts, u32 slot) {
  sol::variadic_results res;
  if (slot >= stage::stage_scene::MAX_BOSSES) {
    res.push_back({ts, sol::nil});
    return res;
  }

  auto& boss = _scene->get_boss(slot);
  if (!boss.is_active()) {
    res.push_back({ts, sol::nil});
    return res;
  }

  res.push_back({ts, sol::in_place_type<lua_boss>, slot});
  return res;
}

static auto parse_proj_args(sol::table& args) -> expect<stage::projectile_args> {
  const auto parse_vec2 = [&](const char* name) -> ntf::optional<vec2> {
    auto lua_vec = args[name].get<sol::optional<sol::table>>();
    if (!lua_vec.has_value()) {
      return {ntf::nullopt};
    }
    auto pos_x = (*lua_vec)["x"].get<sol::optional<f32>>();
    if (!pos_x.has_value()) {
      return {ntf::nullopt};
    }
    auto pos_y = (*lua_vec)["y"].get<sol::optional<f32>>();
    if (!pos_y.has_value()) {
      return {ntf::nullopt};
    }
    return {ntf::in_place, *pos_x, *pos_y};
  };

  auto pos = parse_vec2("pos");
  if (!pos.has_value()) {
    return {ntf::unexpect, "No position"};
  }

  auto vel = parse_vec2("vel");
  if (!vel.has_value()) {
    return {ntf::unexpect, "No velocity"};
  }

  auto sprite_arg = args["sprite"].get<sol::optional<lua_sprite>>();
  if (!sprite_arg.has_value()) {
    return {ntf::unexpect, "No sprite"};
  }
  auto sprite = sprite_arg->get();

  const real ang_speed = args["angular_speed"].get_or(0.f);
  const auto movement = args["movement"].get_or_create<stage::entity_movement>();

  auto lua_state_handler = args["state_handler"].get<sol::optional<sol::protected_function>>();
  ntf::optional<sol::coroutine> state_handler;
  if (lua_state_handler.has_value()) {
    state_handler.emplace(std::move(*lua_state_handler));
  }

  return {ntf::in_place, *pos, *vel, ang_speed, sprite, movement, std::move(state_handler)};
}

sol::variadic_results lua_stage::spawn_proj(sol::this_state ts, sol::table args) {
  sol::variadic_results res;

  auto proj = parse_proj_args(args).transform(
    [&](stage::projectile_args&& args) { return get_scene().spawn_projectile(args); });
  if (!proj.has_value()) {
    res.push_back({ts, sol::nil});
  } else {
    res.push_back({ts, sol::in_place_type<lua_projectile>, *proj});
  }
  return res;
}

sol::table lua_stage::spawn_proj_n(sol::this_state ts, u32 count, sol::protected_function func) {
  sol::state_view lua = ts;

  std::vector<sol::table> args;
  args.reserve(count);
  for (u32 i = 0; i < count; ++i) {
    auto arg_tbl = func.call<sol::table>();
    if (!arg_tbl.valid()) {
      continue;
    }
    args.emplace_back(arg_tbl);
  }

  sol::table out = lua.create_table(count);
  u32 i = 1;
  for (auto& arg : args) {
    auto proj = parse_proj_args(arg).transform(
      [&](stage::projectile_args&& args) { return get_scene().spawn_projectile(args); });
    if (proj.has_value()) {
      out[i] = *proj;
      ++i;
    }
  }
  return out;
}

static fn prep_stage_usertype(sol::table& module) {
  // clang-format off
  return module.new_usertype<lua_stage>(
    "stage", sol::no_constructor,
    "yield", sol::yielding(+[](lua_stage& stage) { stage.on_yield(1); }),
    "yield_ticks", sol::yielding(+[](lua_stage& stage, u32 ticks) { stage.on_yield(ticks); }),
    "yield_secs", sol::yielding(+[](lua_stage& stage, f32 secs) {
      stage.on_yield(secs_to_ticks(secs));
    }),
    "trigger_dialog", &lua_stage::trigger_dialog,
    "get_player", &lua_stage::get_player,
    "get_boss", &lua_stage::get_boss,
    "spawn_proj", &lua_stage::spawn_proj,
    "spawn_proj_n", &lua_stage::spawn_proj_n
  );
  // clang-format on
}

sol::table setup_stage_module(sol::table& okuu_lib) {
  sol::table stage_module = okuu_lib["stage"].get_or_create<sol::table>();
  prep_player_usertype(stage_module);
  prep_boss_usertype(stage_module);
  prep_proj_usertype(stage_module);
  prep_stage_usertype(stage_module);
  return stage_module;
}

} // namespace okuu::lua

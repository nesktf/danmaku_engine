#include "./stage.hpp"
#include "./assets.hpp"

namespace okuu::lua {

lua_player::lua_player() {}

lua_boss::lua_boss(u32 slot) noexcept : _boss_slot{slot} {}

u32 lua_boss::get_slot() const {
  return _boss_slot;
}

bool lua_boss::is_alive(sol::this_state ts) const {
  auto& stage = lua_stage::instance(ts);
  return stage.get_boss(_boss_slot).is_active();
}

void lua_boss::kill(sol::this_state ts) const {
  auto& stage = lua_stage::instance(ts);
  stage.kill_boss(_boss_slot);
}

lua_projectile::lua_projectile(u64 handle) : _handle{handle} {}

u32 lua_projectile::get_handle() const {
  return _handle;
}

bool lua_projectile::is_alive(sol::this_state ts) const {
  auto& stage = lua_stage::instance(ts);
  return stage.is_projectile_alive(_handle);
}

void lua_projectile::kill(sol::this_state ts) const {
  auto& stage = lua_stage::instance(ts);
  stage.kill_projectile(_handle);
}

void lua_projectile::set_pos(sol::this_state ts, f32 x, f32 y) {
  auto& stage = lua_stage::instance(ts);
  stage.set_proj_pos(_handle, x, y);
}

vec2 lua_projectile::get_pos(sol::this_state ts) {
  auto& stage = lua_stage::instance(ts);
  return stage.get_proj_pos(_handle);
}

void lua_projectile::set_movement(sol::this_state ts, stage::entity_movement movement) {
  auto& stage = lua_stage::instance(ts);
  stage.set_proj_mov(_handle, movement);
}

lua_stage::lua_stage(stage::stage_scene& scene) : _scene{scene} {}

void lua_stage::on_yield(u32 ticks) {
  _scene->task_wait(ticks);
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
  auto scale = parse_vec2("scale");

  auto sprite_arg = args["sprite"].get<sol::optional<lua_sprite>>();
  if (!sprite_arg.has_value()) {
    return {ntf::unexpect, "No sprite"};
  }
  auto [atlas, sprite] = sprite_arg->get();

  const real ang_speed = args["angular_speed"].get_or(0.f);
  const auto movement = args["movement"].get<sol::optional<stage::entity_movement>>();

  auto lua_state_handler = args["state_handler"].get<sol::optional<sol::protected_function>>();
  ntf::optional<sol::coroutine> state_handler;
  if (lua_state_handler.has_value()) {
    state_handler.emplace(std::move(*lua_state_handler));
  }

  return {ntf::in_place,
          *pos,
          *vel,
          scale.value_or(vec2{10.f, 10.f}),
          ang_speed,
          std::make_tuple(atlas, sprite, vec2{1.f, 1.f}),
          movement.value_or(stage::entity_movement{}),
          std::move(state_handler)};
}

sol::variadic_results lua_stage::spawn_proj(sol::this_state ts, sol::table args) {
  sol::variadic_results res;

  auto proj = parse_proj_args(args).transform([&](stage::projectile_args&& args) {
    return lua_stage::instance(ts).spawn_projectile(std::move(args));
  });
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
    auto arg_tbl = func.call<sol::table>(i + 1);
    if (!arg_tbl.valid()) {
      continue;
    }
    args.emplace_back(arg_tbl);
  }

  sol::table out = lua.create_table(count);
  u32 i = 1;
  for (auto& arg : args) {
    auto proj = parse_proj_args(arg).transform([&](stage::projectile_args&& args) {
      return lua_stage::instance(ts).spawn_projectile(std::move(args));
    });
    if (proj.has_value()) {
      out[i] = *proj;
      ++i;
    }
  }
  return out;
}

namespace {

fn prep_usertypes(sol::table& module) {
  // clang-format off
  module.new_usertype<lua_player>(
    "player", sol::no_constructor,
    "set_pos", +[](sol::this_state ts, lua_player&, f32 x, f32 y) {
      auto& player = lua_stage::instance(ts).get_player();
      player.pos(x, y);
    },
    "get_pos", +[](sol::this_state ts, lua_player&) -> vec2 {
      auto& player = lua_stage::instance(ts).get_player();
      return player.pos();
    }
  );
  module.new_usertype<lua_boss>(
    "boss", sol::no_constructor,
    "get_slot", &lua_boss::get_slot,
    "kill", &lua_boss::kill,
    "is_alive", &lua_boss::is_alive
  );
  module.new_usertype<lua_projectile>(
    "projectile", sol::no_constructor,
    "is_alive", &lua_projectile::is_alive,
    "kill", &lua_projectile::kill,
    "set_pos", &lua_projectile::set_pos,
    "set_movement", &lua_projectile::set_movement,
    "get_pos", &lua_projectile::get_pos
  );
  module.new_usertype<stage::entity_movement>(
    "movement", sol::no_constructor,
    "move_linear", +[](f32 vel_x, f32 vel_y) {
      return stage::entity_movement::move_linear({vel_x, vel_y});
    },
    "move_towards", +[](f32 vel_x, f32 vel_y, f32 x, f32 y) {
      static constexpr f32 DT = 1/60.f;
      return stage::entity_movement::move_towards({x, y}, {DT*vel_x, DT*vel_y}, {DT, 0.f}, .8f);
    }
  );
  module.new_usertype<lua_stage>(
    "stage", sol::no_constructor,
    "yield", sol::yielding(+[](lua_stage& stage) { stage.on_yield(1); }),
    "yield_ticks", sol::yielding(+[](lua_stage& stage, u32 ticks) { stage.on_yield(ticks); }),
    "yield_secs", sol::yielding(+[](lua_stage& stage, f32 secs) {
      stage.on_yield(secs_to_ticks(secs));
    }),
    "trigger_dialog", &lua_stage::trigger_dialog,
    "get_player", +[](lua_stage&) -> lua_player { return {}; },
    "get_boss", &lua_stage::get_boss,
    "spawn_proj", &lua_stage::spawn_proj,
    "spawn_proj_n", &lua_stage::spawn_proj_n
  );
  // clang-format on
}

} // namespace

sol::table lua_stage::setup_module(sol::table& okuu_lib, stage::stage_scene& scene) {
  sol::table stage_module = okuu_lib["stage"].get_or_create<sol::table>();
  okuu_lib["__curr_stage"] = lua_stage{scene};
  prep_usertypes(stage_module);
  return stage_module;
}

stage::stage_scene& lua_stage::instance(sol::state_view lua) {
  lua_stage stage = lua["okuu"]["__curr_stage"].get<lua_stage>();
  return stage.get();
}

} // namespace okuu::lua

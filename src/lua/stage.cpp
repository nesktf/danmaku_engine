#include "./stage.hpp"
#include "./assets.hpp"
#include "./stage_env.hpp"

namespace okuu::lua {

lua_player::lua_player() {}

lua_boss::lua_boss(u32 slot) noexcept : _boss_slot{slot} {}

u32 lua_boss::get_slot() const {
  return _boss_slot;
}

bool lua_boss::is_alive(sol::this_state ts) const {
  auto& scene = lua_stage::instance(ts)->scene();
  return scene.get_boss(_boss_slot).is_active();
}

void lua_boss::kill(sol::this_state ts) const {
  auto& scene = lua_stage::instance(ts)->scene();
  scene.kill_boss(_boss_slot);
}

lua_projectile::lua_projectile(u64 handle) : _handle{handle} {}

u32 lua_projectile::get_handle() const {
  return _handle;
}

bool lua_projectile::is_alive(sol::this_state ts) const {
  auto& scene = lua_stage::instance(ts)->scene();
  return scene.get_projectiles().is_alive(_handle);
}

void lua_projectile::kill(sol::this_state ts) const {
  auto& scene = lua_stage::instance(ts)->scene();
  scene.get_projectiles().kill(_handle);
}

void lua_projectile::set_pos(sol::this_state ts, f32 x, f32 y) {
  auto& scene = lua_stage::instance(ts)->scene();
  scene.get_projectiles().at(_handle).pos(x, y);
}

vec2 lua_projectile::get_pos(sol::this_state ts) {
  auto& scene = lua_stage::instance(ts)->scene();
  return scene.get_projectiles().at(_handle).pos();
}

void lua_projectile::set_movement(sol::this_state ts, stage::entity_movement movement) {
  auto& scene = lua_stage::instance(ts)->scene();
  scene.get_projectiles().at(_handle).movement(movement);
}

lua_stage::lua_stage(stage_env& env) : _env{env} {}

void lua_stage::on_yield(u32 ticks) {
  _env->scene().task_wait(ticks);
}

sol::variadic_results lua_stage::get_boss(sol::this_state ts, u32 slot) {
  sol::variadic_results res;
  if (slot >= stage::stage_scene::MAX_BOSSES) {
    res.push_back({ts, sol::nil});
    return res;
  }

  auto& boss = _env->scene().get_boss(slot);
  if (!boss.is_active()) {
    res.push_back({ts, sol::nil});
    return res;
  }

  res.push_back({ts, sol::in_place_type<lua_boss>, slot});
  return res;
}

namespace {

auto parse_vec2(sol::table& args, const char* name) -> ntf::optional<vec2> {
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
}

auto parse_proj_args(sol::table& args) -> expect<stage::projectile_args> {
  auto pos = parse_vec2(args, "pos");
  if (!pos.has_value()) {
    return {ntf::unexpect, "No position"};
  }

  auto vel = parse_vec2(args, "vel");
  if (!vel.has_value()) {
    return {ntf::unexpect, "No velocity"};
  }
  auto scale = parse_vec2(args, "scale");

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

} // namespace

sol::variadic_results lua_stage::spawn_proj(sol::this_state ts, sol::table args) {
  sol::variadic_results res;

  auto proj = parse_proj_args(args).transform([&](stage::projectile_args&& args) {
    auto& scene = lua_stage::instance(ts)->scene();
    return scene.get_projectiles().spawn(std::move(args));
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
      auto& scene = lua_stage::instance(ts)->scene();
      return scene.get_projectiles().spawn(std::move(args));
    });
    if (proj.has_value()) {
      out[i] = *proj;
      ++i;
    }
  }
  return out;
}

namespace {

auto parse_sprite_args(sol::table& args) -> expect<stage::sprite_args> {
  auto pos = parse_vec2(args, "pos");
  if (!pos.has_value()) {
    return {ntf::unexpect, "No position"};
  }

  auto vel = parse_vec2(args, "vel");
  if (!vel.has_value()) {
    return {ntf::unexpect, "No velocity"};
  }
  auto scale = parse_vec2(args, "scale");

  auto sprite_arg = args["sprite"].get<sol::optional<lua_sprite>>();
  if (!sprite_arg.has_value()) {
    return {ntf::unexpect, "No sprite"};
  }
  auto [atlas, sprite] = sprite_arg->get();

  const real rot = args["rot"].get_or(0.f);
  const real ang_speed = args["angular_speed"].get_or(0.f);
  const auto movement = args["movement"].get<sol::optional<stage::entity_movement>>();

  return {ntf::in_place,
          *pos,
          *scale,
          rot,
          ang_speed,
          std::make_tuple(atlas, sprite, vec2{1.f, 1.f}),
          movement.value_or(stage::entity_movement{})};
}

} // namespace

sol::variadic_results lua_stage::spawn_sprite(sol::this_state ts, sol::table args) {
  sol::variadic_results res;

  auto ent = parse_sprite_args(args).transform([&](stage::sprite_args&& args) {
    auto& scene = lua_stage::instance(ts)->scene();
    return scene.get_sprites().spawn(std::move(args));
  });
  if (!ent.has_value()) {
    res.push_back({ts, sol::nil});
  } else {
    res.push_back({ts, sol::in_place_type<lua_sprite_ent>, *ent});
  }

  return res;
}

lua_sprite_ent::lua_sprite_ent(u64 handle) : _handle{handle} {}

bool lua_sprite_ent::is_alive(sol::this_state ts) const {
  auto& scene = lua_stage::instance(ts)->scene();
  return scene.get_sprites().is_alive(_handle);
}

void lua_sprite_ent::kill(sol::this_state ts) const {
  auto& scene = lua_stage::instance(ts)->scene();
  scene.get_sprites().kill(_handle);
}

void lua_sprite_ent::set_pos(sol::this_state ts, f32 x, f32 y) {
  auto& scene = lua_stage::instance(ts)->scene();
  scene.get_sprites().at(_handle).pos(x, y);
}

vec2 lua_sprite_ent::get_pos(sol::this_state ts) {
  auto& scene = lua_stage::instance(ts)->scene();
  return scene.get_sprites().at(_handle).pos();
}

void lua_sprite_ent::set_movement(sol::this_state ts, stage::entity_movement movement) {
  auto& scene = lua_stage::instance(ts)->scene();
  scene.get_sprites().at(_handle).set_movement(movement);
}

auto lua_stage::register_event(std::string name, sol::protected_function func) -> lua_event {
  auto it = _env->register_event(std::move(name), std::move(func));
  return {it};
}

void lua_stage::trigger_event(std::string name, sol::variadic_args args) {
  _env->trigger_event(std::move(name), std::move(args));
}

void lua_stage::unregister_event(std::string name, lua_event event) {
  _env->unregister_event(std::move(name), event.get());
}

void lua_stage::clear_events(std::string name) {
  _env->clear_events(std::move(name));
}

namespace {

fn prep_usertypes(sol::table& module) {
  // clang-format off
  module.new_usertype<lua_player>(
    "player", sol::no_constructor,
    "set_pos", +[](sol::this_state ts, lua_player&, f32 x, f32 y) {
      auto& player = lua_stage::instance(ts)->scene().get_player();
      player.pos(x, y);
    },
    "get_pos", +[](sol::this_state ts, lua_player&) -> vec2 {
      auto& player = lua_stage::instance(ts)->scene().get_player();
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
  module.new_usertype<lua_sprite_ent>(
    "sprite_ent", sol::no_constructor,
    "is_alive", &lua_sprite_ent::is_alive,
    "kill", &lua_sprite_ent::kill,
    "set_pos", &lua_sprite_ent::set_pos,
    "set_movement", &lua_sprite_ent::set_movement,
    "get_pos", &lua_sprite_ent::get_pos
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
  module.new_usertype<lua_event>(
    "event", sol::no_constructor
  );
  module.new_usertype<lua_stage>(
    "stage", sol::no_constructor,
    "yield", sol::yielding(+[](lua_stage& stage) { stage.on_yield(1); }),
    "yield_ticks", sol::yielding(+[](lua_stage& stage, u32 ticks) { stage.on_yield(ticks); }),
    "yield_secs", sol::yielding(+[](lua_stage& stage, f32 secs) {
      stage.on_yield(secs_to_ticks(secs));
    }),
    "trigger_event", &lua_stage::trigger_event,
    "register_event", &lua_stage::register_event,
    "unregister_event", &lua_stage::unregister_event,
    "clear_events", &lua_stage::clear_events,
    "get_player", +[](lua_stage&) -> lua_player { return {}; },
    "get_boss", &lua_stage::get_boss,
    "spawn_proj", &lua_stage::spawn_proj,
    "spawn_proj_n", &lua_stage::spawn_proj_n,
    "spawn_sprite", &lua_stage::spawn_sprite
  );
  // clang-format on
}

} // namespace

lua_stage lua_stage::setup_module(sol::table& okuu_lib, stage_env& scene) {
  sol::table stage_module = okuu_lib["stage"].get_or_create<sol::table>();
  lua_stage env_stage{scene};
  okuu_lib["__curr_stage"] = env_stage;
  prep_usertypes(stage_module);
  return env_stage;
}

lua_stage lua_stage::instance(sol::state_view lua) {
  return lua["okuu"]["__curr_stage"].get<lua_stage>();
}

} // namespace okuu::lua

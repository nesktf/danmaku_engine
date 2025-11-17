#include "./lua_env.hpp"

namespace okuu::stage {

namespace {

class lua_player {
public:
  lua_player(player_entity& player) noexcept : _player{player} {}

private:
  ntf::weak_ptr<player_entity> _player;
};

fn prep_player_usertype(sol::table& module) {
  // clang-format off
  return module.new_usertype<lua_player>(
    "player", sol::no_constructor
  );
  // clang-format on
}

class lua_boss {
public:
  lua_boss(ntf::nullable<boss_entity>& boss, u32 slot) noexcept : _boss{boss}, _slot{slot} {}

public:
  u32 get_slot() { return _slot; }

  void kill() {
    if (_boss->has_value()) {
      _boss->reset();
    }
  }

  bool is_alive() { return _boss->has_value(); }

private:
  ntf::weak_ptr<ntf::nullable<boss_entity>> _boss;
  u32 _slot;
};

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

class lua_proj {
  using elem_type = util::free_list<projectile_entity>::element;

public:
  lua_proj(elem_type proj) : _proj{proj} {}

public:
  u32 get_ticks() { return 0; }

  bool is_alive() { return _proj.is_alive(); }

  void kill() { _proj.kill(); }

private:
  elem_type _proj;
};

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

class lua_stage {
public:
  lua_stage(stage_scene& scene) noexcept : _scene{scene} {}

public:
  void yield() { _scene->task_wait_ticks++; }

  void yield_ticks(u32 ticks) { _scene->task_wait_ticks += ticks; }

  void yield_secs(f32 secs) {
    const auto ticks = std::floor(secs * okuu::GAME_UPS);
    _scene->task_wait_ticks += static_cast<u32>(ticks);
  }

  void trigger_dialog(std::string id) {}

  lua_player get_player() { return _scene->player; }

  sol::variadic_results get_boss(sol::this_state ts, u32 slot) {
    sol::variadic_results res;
    if (slot > _scene->boss_count) {
      res.push_back({ts, sol::nil});
      return res;
    }
    auto& boss = _scene->bosses[slot];
    if (!boss.has_value()) {
      res.push_back({ts, sol::nil});
      return res;
    }

    res.push_back({ts, sol::in_place_type<lua_boss>, boss, slot});
    return res;
  };

  sol::variadic_results spawn_proj(sol::this_state ts, sol::table args) {
    sol::variadic_results res;
    auto proj = _actually_spawn_proj(args);
    if (!proj.has_value()) {
      res.push_back({ts, sol::nil});
    } else {
      res.push_back({ts, sol::in_place_type<lua_proj>, *proj});
    }
    return res;
  }

  sol::table spawn_proj_n(sol::this_state ts, u32 count, sol::protected_function func) {
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
      auto proj = _actually_spawn_proj(arg);
      if (proj.has_value()) {
        out[i] = *proj;
        ++i;
      }
    }
    return out;
  }

private:
  ntf::optional<lua_proj> _actually_spawn_proj(sol::table args) { return {ntf::nullopt}; }

private:
  ntf::weak_ptr<stage_scene> _scene;
};

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

sol::table lua_env::init_stage(sol::state_view lua, ntf::weak_ptr<stage_scene> scene) {
  sol::table module = lua.create_table();
  prep_player_usertype(module);
  prep_boss_usertype(module);
  prep_proj_usertype(module);
  prep_stage_usertype(module);
  return module;
}

} // namespace okuu::stage

#include "./lua_env.hpp"

namespace okuu::stage {

namespace {

constexpr std::string_view incl_path = ";res/script/?.lua";

using module_pair = std::pair<const char*, void (*)(sol::table&)>;
constexpr module_pair modules[]{
  {"log",
   +[](sol::table& module) {
   }},
  {"math",
   +[](sol::table& module) {
   }},
};
constexpr std::size_t module_count = sizeof(modules) / sizeof(module_pair);
constexpr std::string_view stlib_key = "okuu";

sol::table prepare_lua_env(sol::state_view lua, ntf::weak_ptr<stage_scene> scene) {
  lua.open_libraries(sol::lib::base, sol::lib::coroutine, sol::lib::package, sol::lib::table,
                     sol::lib::math, sol::lib::string);

  lua["package"]["path"] = incl_path.data();

  auto lib = lua[stlib_key.data()].get_or_create<sol::table>();
  for (size_t i = 0; i < module_count; ++i) {
    const auto& [name, loader] = modules[i];
    auto module = lib[name].get_or_create<sol::table>();
    loader(module);
  }

  static constexpr real DT = 1.f / 60.f;
  auto lib_stage = lib["stage"].get_or_create<sol::table>();
  lib_stage.set_function("spawn_boss", [=](float scale, cmplx p0, cmplx p1) {
    if (scene->boss_count >= stage_scene::MAX_BOSSES) {
      ntf::logger::warning("Over the boss limit");
      return;
    }

    // auto [atlas_idx, sprite] = scene->find_sprite("ball_solid").value();
    // const auto& atlas = scene->atlas_assets[atlas_idx];
    // entity_sprite boss_sprite{atlas, sprite};
    // const vec2 boss_pos{p0.real(), p0.imag()};
    // const auto mov = entity_movement::move_towards({p1.real(), p1.imag()}, DT *
    // vec2{10.f, 10.f},
    //                                                vec2{DT, DT}, .8f);
    //
    // const u32 boss_idx = scene->boss_count;
    // scene->bosses[boss_idx].emplace(0u, boss_pos, boss_sprite, mov);
    scene->boss_count++;
  });

  lib_stage.set_function("move_boss", [=](sol::table args) {
    if (scene->boss_count == 0) {
      return;
    }
    if (!args["pos"].is<cmplx>()) {
      return;
    }
    const cmplx pos = args["pos"].get<cmplx>();

    const u32 other_idx = scene->boss_count - 1;
    const u32 boss_idx = args.get_or<u32>("boss_idx", other_idx); // the last boss
    if (boss_idx >= scene->boss_count) {
      return;
    }
    auto& boss = scene->bosses[boss_idx];
    NTF_ASSERT(boss.has_value());
    boss->set_movement(entity_movement::move_towards({pos.real(), pos.imag()},
                                                     DT * vec2{10.f, 10.f}, vec2{DT, DT}, .8f));
  });

  lib_stage.set_function("boss_pos", [=](sol::table args) -> cmplx {
    if (scene->boss_count == 0) {
      return {};
    }
    const u32 other_idx = scene->boss_count - 1;
    const u32 boss_idx = args.get_or<u32>("boss_idx", other_idx); // the last boss
    if (boss_idx >= scene->boss_count) {
      return {};
    }
    auto& boss = scene->bosses[boss_idx];
    NTF_ASSERT(boss.has_value());
    return {boss->pos().x, boss->pos().y};
  });

  lib_stage.set_function("player_pos", [=]() -> cmplx {
    return {scene->player.pos().x, scene->player.pos().y};
  });

  lib_stage.set_function("cowait",
                         sol::yielding([=](u32 time) { scene->task_wait_ticks = time; }));

  auto mov_type = lib_stage.new_usertype<entity_movement>(
    "move", sol::no_constructor, "make_linear", +[](sol::table tbl) {
      const vec2 vel{tbl.get<real>("real"), tbl.get<real>("imag")};
      return entity_movement::move_linear(vel);
    });

  auto proj_type = lib_stage.new_usertype<projectile_entity>(
    "proj", sol::no_constructor, "movement", sol::property(&projectile_entity::set_movement),
    "angular_speed",
    sol::property(&projectile_entity::angular_speed, &projectile_entity::set_angular_speed));

  auto view_type = lib_stage.new_usertype<lua_env::projectile_view>(
    "pview", sol::no_constructor, "make",
    [=](size_t size) -> lua_env::projectile_view {

    },
    "size", sol::property(&lua_env::projectile_view::size), "for_each",
    &lua_env::projectile_view::for_each);

  return lib;
}

} // namespace

expect<lua_env> lua_env::load(const std::string& script_path,
                              std::unique_ptr<stage_scene>&& scene) {
  // auto vp = okuu::render::stage_viewport::create(600, 700, 640, 360);

  sol::state lua;
  auto lib_table = prepare_lua_env(lua, scene.get());
  lua.script_file(script_path);

  if (!lib_table["init"].is<sol::function>()) {
    return {ntf::unexpect, "Init function not defined in stage script"};
  }
  if (!lib_table["main"].is<sol::function>()) {
    return {ntf::unexpect, "Main function not defined in stage script"};
  }

  return {ntf::in_place, std::move(lua), std::move(lib_table), std::move(scene)};
}

lua_env::lua_env(sol::state&& lua, sol::table lib_table, std::unique_ptr<stage_scene>&& scene) :
    _lua{std::move(lua)}, _task_thread{sol::thread::create(_lua.lua_state())},
    _task{_task_thread.state(), lib_table["main"].get<sol::function>()}, _scene{std::move(scene)} {
}

lua_env::projectile_view::projectile_view(util::free_list<projectile_entity>& list, size_t count) :
    _list{list} {
  _items.reserve(count);
  for (u32 i = 0; i < count; ++i) {
    // auto elem = list.request_elem(0, vec2{0.f, 0.f}, vec2{1.f, 1.f}, M_PIf, sprite);
    // _items.emplace_back(elem.idx());
  }
}

void lua_env::projectile_view::for_each(sol::function f) {
  for (u32 item : _items) {
    util::free_list<projectile_entity>::element elem{*_list, item};
    if (elem.alive()) {
      f(*elem);
    }
  }
}

void lua_env::tick() {
  scene().tick();
}

} // namespace okuu::stage

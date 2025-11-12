#include "./stage.hpp"

#include "../render/instance.hpp"
#include "../render/stage.hpp"
#include <ntfstl/logger.hpp>

namespace okuu::stage {

namespace {

constexpr std::string_view incl_path = ";res/script/?.lua";

using module_pair = std::pair<const char*, void (*)(sol::table&)>;
constexpr module_pair modules[]{
  {"log",
   +[](sol::table& module) {
     module.set_function("error", [](std::string msg) { ntf::logger::error("{}", msg); });
     module.set_function("warn", [](std::string msg) { ntf::logger::warning("{}", msg); });
     module.set_function("debug", [](std::string msg) { ntf::logger::debug("{}", msg); });
     module.set_function("info", [](std::string msg) { ntf::logger::info("{}", msg); });
     module.set_function("verbose", [](std::string msg) { ntf::logger::info("{}", msg); });
   }},
  {"math",
   +[](sol::table& module) {
     auto cmplx_type = module.new_usertype<cmplx>(
       "cmplx", sol::call_constructor,
       sol::constructors<cmplx(), cmplx(float), cmplx(float, float)>{}, "real",
       sol::property(
         +[](cmplx& c, float f) { c.real(f); }, +[](cmplx& c) -> float { return c.real(); }),
       "imag",
       sol::property(
         +[](cmplx& c, float f) { c.imag(f); }, +[](cmplx& c) -> float { return c.imag(); }),
       "expi", &shogle::expic, "polar", &std::polar<float>,

       sol::meta_function::addition,
       sol::resolve<cmplx(const cmplx&, const cmplx&)>(&std::operator+),
       sol::meta_function::subtraction,
       sol::resolve<cmplx(const cmplx&, const cmplx&)>(&std::operator-),
       sol::meta_function::multiplication,
       sol::overload(sol::resolve<cmplx(const cmplx&, const cmplx&)>(&std::operator*),
                     sol::resolve<cmplx(const cmplx&, const float&)>(&std::operator*),
                     sol::resolve<cmplx(const float&, const cmplx&)>(&std::operator*)),
       sol::meta_function::division,
       sol::overload(sol::resolve<cmplx(const cmplx&, const cmplx&)>(&std::operator/),
                     sol::resolve<cmplx(const cmplx&, const float&)>(&std::operator/),
                     sol::resolve<cmplx(const float&, const cmplx&)>(&std::operator/)));
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

    auto [atlas_idx, sprite] = scene->find_sprite("ball_solid").value();
    const auto& atlas = scene->atlas_assets[atlas_idx];
    entity_sprite boss_sprite{atlas, sprite};
    const vec2 boss_pos{p0.real(), p0.imag()};
    const auto mov = entity_movement::move_towards({p1.real(), p1.imag()}, DT * vec2{10.f, 10.f},
                                                   vec2{DT, DT}, .8f);

    const u32 boss_idx = scene->boss_count;
    scene->bosses[boss_idx].emplace(0u, boss_pos, boss_sprite, mov);
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

  lib_stage.set_function(
    "player_pos", [=]() -> cmplx { return {scene->player.pos().x, scene->player.pos().y}; });

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

stage_scene::stage_scene(u32 max_entities, player_entity&& player_,
                         std::vector<assets::sprite_atlas>&& atlas_assets_) :
    projs{}, bosses{}, boss_count{0u}, player{std::move(player_)},
    atlas_assets{std::move(atlas_assets_)},
    renderer{okuu::render::stage_renderer::create(max_entities).value()}, task_wait_ticks{0u},
    ticks{0u} {}

auto stage_scene::find_sprite(std::string_view name) const
  -> ntf::optional<idx_elem<assets::sprite_atlas::sprite>> {

  for (u32 i = 0; const auto& curr : atlas_assets) {
    auto item = curr.find_sprite(name);
    if (item) {
      return {ntf::in_place, i, *item};
    }
    ++i;
  }
  return {ntf::nullopt};
}

auto stage_scene::find_anim(std::string_view name) const
  -> ntf::optional<idx_elem<assets::sprite_atlas::animation>> {

  for (u32 i = 0; const auto& curr : atlas_assets) {
    auto item = curr.find_animation(name);
    if (item) {
      return {ntf::in_place, i, *item};
    }
    ++i;
  }
  return {ntf::nullopt};
}

void stage_scene::render(double dt, double alpha) {
  // The scene has to render the following (in order):
  // - The background
  // - The boss(es)
  // - The player
  // - The items
  // - The danmaku
  NTF_UNUSED(dt);
  NTF_UNUSED(alpha);

  const auto render_sprite = [&](auto& entity) {
    const auto [atlas, idx] = entity.sprite();
    const auto [tex, uvs] = atlas->render_data(idx);
    this->renderer.enqueue_sprite({
      .transform = entity.transform(),
      .texture = tex,
      .ticks = this->ticks,
      .uvs = uvs,
      .color = {1.f, 1.f, 1.f, 1.f},
    });
  };

  // for (u32 i = 0; i < this->boss_count; ++i) {
  //   const auto& boss = bosses[i];
  //   NTF_ASSERT(boss.has_value());
  //   render_sprite(*boss);
  // }

  render_sprite(player);

  // const auto proj_span = projs.elems();
  // for (const auto& proj : proj_span) {
  //   if (!proj.has_value()) {
  //     continue;
  //   }
  //   render_sprite(*proj);
  // }
  render::render_stage(this->renderer);
  render::render_viewport(this->renderer.viewport(),
                          shogle::framebuffer::get_default(render::g_renderer->ctx));
}

void stage_scene::tick() {
  player.tick();

  for (u32 i = 0; i < this->boss_count; ++i) {
    auto& boss = bosses[i];
    if (boss.has_value()) {
      boss->tick();
    }
  }
}

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

// namespace stage {
//
// context::context(std::string_view script_path) {
//   auto fb_shader = res::get_shader("framebuffer").value();
//   _viewport.init(VIEWPORT, VIEWPORT / 2, (vec2)render::win_size() * .5f, fb_shader);
//   _vp_sub = render::vp_subscribe(
//     [this](std::size_t w, std::size_t h) { _viewport.update_pos(vec2{w, h} * .5f); });
//
//   auto stlib = package::stlib_open(_lua);
//   _lua_post_open(stlib);
//   _prepare_player();
//   _lua.script_file(script_path.data());
//
//   if (stlib["init"].is<sol::function>()) {
//     stlib["init"].get<sol::function>()();
//   } else {
//     logger::warning("[stage::context] {}.init() not defined", stlib_key);
//   }
//
//   if (res::has_requests()) {
//     res::start_loading([this]() { start(); });
//   } else {
//     start();
//   }
// }
//
// context::~context() noexcept {
//   render::vp_unsuscribe(_vp_sub);
//   _viewport.destroy();
// }
//
// void context::start() {
//   NTF_ASSERT(_state == context::state::loading);
//
//   auto stlib = _lua[stlib_key.data()].get<sol::table>();
//
//   if (!stlib["main"].is<sol::function>()) {
//     throw ntf::error{"[stage::context] {}.main() not defined", stlib_key};
//   }
//   auto main_task = stlib["main"].get<sol::function>();
//   _task_thread = sol::thread::create(_lua.lua_state());
//   _task = sol::coroutine{_task_thread.state(), main_task};
//
//   _on_render = stlib["draw"].get<sol::protected_function>();
//   _on_tick = stlib["update"].get<sol::protected_function>();
//
//   _state = context::state::main;
// }
//
// void context::_lua_post_open(sol::table stlib) {
//   auto module = stlib["stage"].get_or_create<sol::table>();
//
//   module.set_function("spawn_boss", [this](float scale, cmplx p0, cmplx p1) {
//     const auto atlas = res::get_atlas("effects").value();
//     const auto entry = atlas->group_at(atlas->find_group("ball_solid").value())[0];
//     const auto aspect = atlas->at(entry).aspect();
//
//     ntf::transform2d transform;
//     transform.pos(p0).scale(scale * aspect);
//
//     _boss = stage::boss{stage::boss::args{
//       .transform = transform,
//       .movement = stage::entity_movement_towards(p1, DT * cmplx{10.f, 10.f}, cmplx{DT}, .8f),
//       .animator = stage::entity_animator_static(atlas, entry),
//     }};
//   });
//
//   module.set_function("move_boss", [this](cmplx pos) {
//     _boss.movement = stage::entity_movement_towards(pos, DT * cmplx{10.f, 10.f}, cmplx{DT},
//     .8f);
//   });
//
//   module.set_function("boss_pos", [this]() -> cmplx { return _boss.transform.cpos(); });
//
//   module.set_function("player_pos", [this]() -> cmplx { return _player.transform.cpos(); });
//
//   module.set_function("spawn_danmaku", [this](cmplx dir, float speed, cmplx pos) {
//     const auto atlas = res::get_atlas("effects").value();
//     const auto entry = atlas->group_at(atlas->find_group("star_med").value())[3];
//     const auto aspect = atlas->at(entry).aspect();
//
//     ntf::transform2d transform;
//     transform.pos(pos).scale(aspect * 20.f);
//
//     _projs.emplace_back(stage::projectile::args{
//       .transform = transform,
//       .movement = stage::entity_movement_linear(dir * speed),
//       .animator = stage::entity_animator_static(atlas, entry),
//       .angular_speed = 2 * M_PIf * DT,
//     });
//   });
//
//   module.set_function("viewport", [this]() -> sol::table {
//     return _lua.create_table_with("x", VIEWPORT.x, "y", VIEWPORT.y);
//   });
//
//   module.set_function("cowait", sol::yielding([this](frame_delay time) {
//                         _task_wait = time;
//                         logger::debug("[stage::context] Task yield {} frames", time);
//                       }));
//
//   auto move_type = module.new_usertype<stage::entity_movement>(
//     "move", sol::no_constructor, "make_linear", +[](sol::table tbl) -> stage::entity_movement {
//       cmplx vel = cmplx{tbl.get<float>("real"), tbl.get<float>("imag")};
//       return stage::entity_movement_linear(vel);
//     });
//
//   auto proj_type = module.new_usertype<stage::projectile>(
//     "proj", sol::no_constructor, "movement", &stage::projectile::movement, "angular_speed",
//     &stage::projectile::angular_speed);
//
//   auto view_type = module.new_usertype<stage::projectile_view>(
//     "pview", sol::no_constructor, "make",
//     [this](std::size_t size) { return projectile_view{_projs, size}; },
//     // "pview", sol::call_constructor, sol::constructors<stage::projectile_view(std::size_t)>{},
//     "size", &stage::projectile_view::size, "for_each", &stage::projectile_view::for_each);
// }
//
// void context::_prepare_player() {
//   const auto atlas = res::get_atlas("chara_cirno").value();
//   const stage::player::animator_type::animation_data anim{
//     atlas->find_sequence("cirno.idle").value(),
//
//     atlas->find_sequence("cirno.left").value(),
//     atlas->find_sequence("cirno.left_to_idle").value(),
//     atlas->find_sequence("cirno.idle_to_left").value(),
//
//     atlas->find_sequence("cirno.right").value(),
//     atlas->find_sequence("cirno.right_to_idle").value(),
//     atlas->find_sequence("cirno.idle_to_right").value(),
//   };
//   const vec2 sprite_aspect = atlas->at(atlas->sequence_at(anim[0])[0]).aspect();
//
//   ntf::transform2d transform;
//   transform.pos((vec2)VIEWPORT * .5f).scale(sprite_aspect * 70.f);
//
//   float sp = 500.f * DT;
//   _player = stage::player{stage::player::args{
//     .transform = transform,
//     .atlas = atlas,
//     .anim = anim,
//     .base_speed = sp,
//     .slow_speed = sp * .66f,
//   }};
// }
//
// void context::tick() {
//   if (_state != context::state::main) {
//     return;
//   }
//
//   if (_task_time >= _task_wait) {
//     _task_time = 0;
//     _task();
//   }
//   ++_task_time;
//
//   if (_on_tick) {
//     _on_tick();
//   }
//
//   for (auto& proj : _projs) {
//     proj.tick();
//   }
//
//   if (!_boss.hide) {
//     _boss.tick();
//   }
//
//   _player.tick();
//
//   std::erase_if(_projs, [](const auto& proj) {
//     const auto pos = proj.transform.pos();
//     const float extra = 10.f;
//     return (pos.x < -extra || pos.y < -extra || pos.x > VIEWPORT.x + extra ||
//             pos.y > VIEWPORT.y + extra);
//   });
//
//   ++_tick_count;
// }
//
// void context::render(double dt, [[maybe_unused]] double alpha) {
//   if (_state != context::state::main) {
//     return;
//   }
//
//   _viewport.bind(render::win_size(), [this, dt, alpha]() {
//     render::clear_viewport();
//
//     if (_on_render) {
//       _on_render(dt, alpha);
//     }
//
//     auto& boss = _boss;
//     if (!boss.hide) {
//       render::draw_sprite(boss.sprite(), boss.mat(), _viewport.proj(), _viewport.view());
//     }
//
//     auto& player = _player;
//     render::draw_sprite(player.sprite(), player.mat(), _viewport.proj(), _viewport.view());
//
//     for (auto& proj : _projs) {
//       render::draw_sprite(proj.sprite(), proj.mat(), _viewport.proj(), _viewport.view());
//     }
//   });
//   render::draw_background(dt);
//   _viewport.draw(render::win_proj());
//   ntf::transform2d text_transform;
//   text_transform.pos(30.f, 100.f).scale(.75f);
//   std::string txt = fmt::format("danmaku: {}", _projs.size());
//   render::draw_text(txt, color4{1.f}, text_transform.mat());
//   auto cpos = _player.transform.pos();
//   txt = fmt::format("cino pos: {} {}", cpos.x, cpos.y);
//   text_transform.pos(30.f, 140.f);
//   render::draw_text(txt, color4{1.f}, text_transform.mat());
//
//   ++_frame_count;
// }
//
// projectile_view::projectile_view(std::list<stage::projectile>& list, std::size_t size) :
//     _size(size) {
//   std::list<stage::projectile> new_list;
//   const auto atlas = res::get_atlas("effects").value();
//   const auto entry = atlas->group_at(atlas->find_group("star_med").value())[1];
//   const auto aspect = atlas->at(entry).aspect();
//
//   ntf::transform2d transform;
//   transform.pos((vec2)VIEWPORT * .5f).scale(aspect * 20.f);
//
//   for (size_t i = 0; i < size; ++i) {
//     new_list.emplace_back(stage::projectile::args{
//       .transform = transform,
//       .movement = stage::entity_movement_linear(cmplx{0.f}),
//       .animator = stage::entity_animator_static(atlas, entry),
//       .clean_flag = false,
//     });
//   }
//   _begin = new_list.begin();
//   list.splice(list.end(), new_list);
// }
//
// void projectile_view::for_each(sol::function f) {
//   auto it = _begin;
//   for (std::size_t i = 0; i < _size; ++i, ++it) {
//     f(*it);
//   }
// }
//
// } // namespace stage

#define SOL_ALL_SAFETIES_ON 1
#include "game/context.hpp"

namespace {

// class projectile_view {
// public:
//   using list = std::list<game::entity::projectile>;
//
// public:
//   projectile_view(std::size_t size) : _size(size) {
//     list projs;
//
//     const auto atlas = res::get_atlas("effects").value();
//     const auto entry = atlas->group_at(atlas->find_group("star_med").value())[1];
//     const auto aspect = atlas->at(entry).aspect();
//
//     ntf::transform2d transform;
//     transform
//       .set_pos((vec2)VIEWPORT*.5f)
//       .set_scale(aspect*20.f);
//
//     for (size_t i = 0; i < size; ++i) {
//       list.emplace_back(stage::projectile::args{
//         .transform = transform,
//         .movement = stage::entity_movement_linear(cmplx{0.f}),
//         .animator = stage::entity_animator_static(atlas, entry),
//         .clean_flag = false,
//       });
//     }
//     _begin = list.begin();
//     stg.projs.splice(stg.projs.end(), list);
//   }
//
//   void for_each(sol::function f) {
//     auto it = _begin;
//     for (std::size_t i = 0; i < _size; ++i, ++it) {
//       f(*it);
//     }
//   }
//
// public:
//   std::size_t size() const { return _size; }
//
// private:
//   iterator _begin;
//   std::size_t _size;
// };

} // namespace


namespace game {

context::context(context_args args) :
  _args(std::move(args)) { _prepare_lua_env(); }

bool context::init(ntf::async_data_loader& loader) {
  // TODO: Handle init errors
  assert(_state == ctx_state::preload);

  auto& script = _args.script;
  _lua.script_file(script);

  sol::protected_function init_task = _lua["__INIT_TASK"];
  if (!init_task) {
    ntf::log::error("[game::context] Lua initial task not defined in script {}", script);
    return false;
  }

  _entrypoint = _lua["__MAIN_TASK"];
  if (!_entrypoint) {
    ntf::log::error("[game::context] Lua main task not defined in script {}", script);
    return false;
  }

  init_task();
  _res.init_requests(loader, [&]() {
    _prepare_player();
    _state = ctx_state::main;
  });
  _state = ctx_state::loading;

  return true;
}

void context::tick() {
  switch (_state) {
    case ctx_state::main: {
      _update_stage();
    };
    default: break;
  }
}

void context::draw(double dt, double alpha, render::stage_viewport& vp) {
  switch (_state) {
    case ctx_state::main: {
      _draw_stage(dt, alpha, vp);
    };
    default: break;
  }
}

void context::_draw_stage([[maybe_unused]] double dt, [[maybe_unused]] double alpha,
                          render::stage_viewport& vp) {
  const auto& proj = vp.proj();
  const auto& view = vp.view();

  vp.bind(render::win_size(), [&]() {
    render::clear_viewport();

    if (!_boss.hide) {
      render::draw_sprite(_boss.sprite(), _boss.mat(), proj, view);
    }

    render::draw_sprite(_player.sprite(), _player.mat(), proj, view);
    
    for (auto& projectile : _projectiles) {
      render::draw_sprite(projectile.sprite(), projectile.mat(), proj, view);
    }
  });
  render::draw_background(dt);
  vp.draw(render::win_proj());

  ntf::transform2d text_transform;
  text_transform
    .set_pos(30.f, 100.f)
    .set_scale(.75f);
  std::string txt = fmt::format("danmaku: {}", _projectiles.size());
  render::draw_text(txt, color4{1.f}, text_transform.mat());
  auto cpos = _player.transform.pos();
  txt = fmt::format("cino pos: {} {}", cpos.x, cpos.y);
  text_transform.set_pos(30.f, 140.f);
  render::draw_text(txt, color4{1.f}, text_transform.mat());

  ++_frame_count;
}

void context::_update_stage() {
  if (_task_time >= _task_wait) {
    _task_time = 0;

    sol::table task_yield = _entrypoint();
    const auto delay = task_yield.get<frame_delay>("wait_time");
    if (delay < 0) {
      ntf::log::debug("[game::context] Lua main task returned");
    } else {
      _task_wait = delay;
    }
  }
  ++_task_time;

  for (auto& proj : _projectiles) {
    proj.tick();
  }

  if (!_boss.hide) {
    _boss.tick();
  }

  _player.tick();

  const float extra = 10.f;
  std::erase_if(_projectiles, [&](const auto& proj) {
    const auto pos = proj.transform.pos();
    return (
      pos.x < -extra || 
      pos.y < -extra || 
      pos.x > VIEWPORT.x+extra || 
      pos.y > VIEWPORT.y+extra
    );
  });

  ++_tick_count;
}

void context::_prepare_player() {
  const auto atlas = _res.atlas_at("chara_cirno");
  const entity::player::animator_type::animation_data anim {
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

  float sp = 500.f*DT;
  _player = entity::player{entity::player::args{
    .transform = transform,
    .atlas = atlas,
    .anim = anim,
    .base_speed = sp,
    .slow_speed = sp*.66f,
  }};
}

void context::_open_lua_libs() {
  _lua.open_libraries(
    sol::lib::base, sol::lib::coroutine,
    sol::lib::package, sol::lib::table,
    sol::lib::math, sol::lib::string
  );

  sol::table module = _lua.create_table();

  auto cmplx_type = module.new_usertype<cmplx>(
    "cmplx", sol::call_constructor,
    sol::constructors<cmplx(), cmplx(float), cmplx(float, float)>{},
    "real", sol::property(
      +[](cmplx& c, float f) { c.real(f); },
      +[](cmplx& c) -> float { return c.real(); }
    ),
    "imag", sol::property(
      +[](cmplx& c, float f) { c.imag(f); },
      +[](cmplx& c) -> float { return c.imag(); }
    ),
    "expi", &ntf::expic,
    "polar", &std::polar<float>,

    sol::meta_function::addition,
      sol::resolve<cmplx(const cmplx&, const cmplx&)>(&std::operator+),
    sol::meta_function::subtraction,
      sol::resolve<cmplx(const cmplx&, const cmplx&)>(&std::operator-),
    sol::meta_function::multiplication, sol::overload(
      sol::resolve<cmplx(const cmplx&, const cmplx&)>(&std::operator*),
      sol::resolve<cmplx(const cmplx&, const float&)>(&std::operator*),
      sol::resolve<cmplx(const float&, const cmplx&)>(&std::operator*)
    ),
    sol::meta_function::division, sol::overload(
      sol::resolve<cmplx(const cmplx&, const cmplx&)>(&std::operator/),
      sol::resolve<cmplx(const cmplx&, const float&)>(&std::operator/),
      sol::resolve<cmplx(const float&, const cmplx&)>(&std::operator/)
    )
  );

  auto move_type = module.new_usertype<game::entity::movement>(
    "movement", sol::no_constructor,
    "linear", &game::entity::movement_linear
  );

  auto proj_type = module.new_usertype<game::entity::projectile>(
    "proj", sol::no_constructor,
    "movement", &game::entity::projectile::movement,
    "angular_speed", &game::entity::projectile::angular_speed
  );

  // auto view_type = module.new_usertype<projectile_view>(
  //   "pview", sol::call_constructor,
  //   sol::constructors<projectile_view(std::size_t)>{},
  //   "size", &projectile_view::size,
  //   "for_each", &projectile_view::for_each
  // );

  _lua.set("funny_lib", module);
}

void context::_prepare_lua_env() {
  _open_lua_libs();

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
    const auto atlas = _res.atlas_at("effects");
    const auto entry = atlas->group_at(atlas->find_group("ball_solid").value())[0];
    const auto aspect = atlas->at(entry).aspect();

    cmplx first = cmplx{p0.get<float>("real"), p0.get<float>("imag")};
    cmplx last = cmplx{p1.get<float>("real"), p1.get<float>("imag")};

    ntf::transform2d transform;
    transform
      .set_pos(first)
      .set_scale(scale*aspect);

    _boss = entity::boss{entity::boss::args{
      .transform = transform,
      .movement = entity::movement_towards(last, DT*cmplx{10.f,10.f}, cmplx{DT}, .8f),
      .animator = entity::animator_static(res::sprite{atlas, entry}),
    }};
  });

  _lua.set_function("__MOVE_BOSS", [this](sol::table pos) {
    cmplx new_pos = cmplx{pos.get<float>("real"), pos.get<float>("imag")};
    _boss.movement = entity::movement_towards(new_pos, DT*cmplx{10.f,10.f}, cmplx{DT}, .8f);
  });

  _lua.set_function("__BOSS_POS", [this]() -> sol::table {
    const auto pos = _boss.transform.pos();
    return _lua.create_table_with("real", pos.x, "imag", pos.y);
  });
  
  // Player functions
  _lua.set_function("__PLAYER_POS", [this]() -> sol::table {
    const auto pos = _player.transform.pos();
    return _lua.create_table_with("x", pos.x, "y", pos.y);
  });

  // Projectile functions
  _lua.set_function("__SPAWN_DANMAKU", [this](sol::table dir, float speed, sol::table pos) {
    cmplx proj_dir = cmplx{dir.get<float>("real"), dir.get<float>("imag")};
    cmplx proj_pos = cmplx{pos.get<float>("real"), pos.get<float>("imag")};

    const auto atlas = _res.atlas_at("effects");
    const auto entry = atlas->group_at(atlas->find_group("star_med").value())[3];
    const auto aspect = atlas->at(entry).aspect();

    ntf::transform2d transform;
    transform
      .set_pos(proj_pos)
      .set_scale(aspect*20.f);

    _projectiles.emplace_back(entity::projectile::args {
      .transform = transform,
      .movement = entity::movement_linear(proj_dir*speed),
      .animator = entity::animator_static(res::sprite{atlas, entry}),
      .angular_speed = 2*M_PIf*DT,
    });
  });

  // Misc
  _lua.set_function("__STAGE_TICKS", [this]() -> frames { return _tick_count; });
  _lua.set_function("__VIEWPORT", [this]() -> sol::table {
    return _lua.create_table_with("x", VIEWPORT.x, "y", VIEWPORT.y);
  });
}

} // namespace game

#define SOL_ALL_SAFETIES_ON 1
#include <sol/sol.hpp>

#include "stage/stage.hpp"

#include "resources.hpp"
#include "render.hpp"

#include <shogle/core/allocator.hpp>

namespace {

static stage::entity_movement move_make_linear(sol::table tbl) {
  cmplx vel = cmplx{tbl.get<float>("real"), tbl.get<float>("imag")};
  return stage::entity_movement_linear(vel);
}

static auto funny_lib(sol::this_state s) -> sol::table {
  sol::state_view lua{s};
  sol::table module = lua.create_table();

  auto move_type = module.new_usertype<stage::entity_movement>(
    "move", sol::no_constructor,
    "make_linear", &move_make_linear
  );

  auto proj_type = module.new_usertype<stage::projectile>(
    "proj", sol::no_constructor,
    "movement", &stage::projectile::movement,
    "angular_speed", &stage::projectile::angular_speed
  );

  auto view_type = module.new_usertype<stage::projectile_view>(
    "pview", sol::call_constructor, sol::constructors<stage::projectile_view(std::size_t)>{},
    "size", &stage::projectile_view::size,
    "for_each", &stage::projectile_view::for_each
  );

  return module;
};

stage::context* _curr_ctx;

} // namespace


namespace stage {

context::context(std::string_view stage_script) {
  auto fb_shader = res::get_shader("framebuffer").value();
  _viewport.init(VIEWPORT, VIEWPORT/2, (vec2)render::win_size()*.5f, fb_shader);
  _vp_sub = render::vp_subscribe([this](std::size_t w, std::size_t h) {
    _viewport.update_pos(vec2{w, h}*.5f);
  });

  _curr_ctx = this;

  _prepare_player();

  _prepare_lua_env();

  _lua.script_file(stage_script.data());

  ntf::log::debug("D");
  auto init_task = _lua.get<sol::protected_function>("__INIT_TASK");
  if (init_task) {
    init_task();
  } else {
    ntf::log::warning("[stage::context] Lua initial task not defined in script {}", stage_script);
  }

  _entrypoint = _lua.get<sol::protected_function>("__MAIN_TASK");
  if (!_entrypoint) {
    throw ntf::error{"[stage::context] Lua main task not defined in script {}", 
      std::string{stage_script}};
  }
}

context::~context() noexcept {
  render::vp_unsuscribe(_vp_sub);
  _viewport.destroy();
}

void context::_prepare_player() {
  const auto atlas = res::get_atlas("chara_cirno").value();
  const stage::player::animator_type::animation_data anim {
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
  _player = stage::player{stage::player::args{
    .transform = transform,
    .atlas = atlas,
    .anim = anim,
    .base_speed = sp,
    .slow_speed = sp*.66f,
  }};
}

void context::_prepare_lua_env() {
  _lua.open_libraries(
    sol::lib::base, sol::lib::coroutine,
    sol::lib::package, sol::lib::table,
    sol::lib::math, sol::lib::string
  );

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

    _boss = stage::boss{stage::boss::args{
      .transform = transform,
      .movement = stage::entity_movement_towards(last, DT*cmplx{10.f,10.f}, cmplx{DT}, .8f),
      .animator = stage::entity_animator_static(atlas, entry),
    }};
  });

  _lua.set_function("__MOVE_BOSS", [this](sol::table pos) {
    cmplx new_pos = cmplx{pos.get<float>("real"), pos.get<float>("imag")};
    _boss.movement = stage::entity_movement_towards(new_pos, DT*cmplx{10.f,10.f}, cmplx{DT}, .8f);
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

    const auto atlas = res::get_atlas("effects").value();
    const auto entry = atlas->group_at(atlas->find_group("star_med").value())[3];
    const auto aspect = atlas->at(entry).aspect();

    ntf::transform2d transform;
    transform
      .set_pos(proj_pos)
      .set_scale(aspect*20.f);

    _projs.emplace_back(stage::projectile::args {
      .transform = transform,
      .movement = stage::entity_movement_linear(proj_dir*speed),
      .animator = stage::entity_animator_static(atlas, entry),
      .angular_speed = 2*M_PIf*DT,
    });
  });

  // Misc
  _lua.set_function("__STAGE_TICKS", [this]() -> frames { return _tick_count; });
  _lua.set_function("__VIEWPORT", [this]() -> sol::table {
    return _lua.create_table_with("x", VIEWPORT.x, "y", VIEWPORT.y);
  });


  _lua.require("funny_lib", sol::c_call<decltype(&funny_lib), &funny_lib>);
}

void context::tick() {
  // auto& lua = _lua;
  // sol::function on_tick = lua["__ON_TICK"];
  // on_tick();

  if (_task_time >= _task_wait) {
    _task_time = 0;

    sol::table task_yield = _entrypoint();
    const auto delay = task_yield.get<frame_delay>("wait_time");
    if (delay < 0) {
      ntf::log::debug("[stage] Lua main task returned");
    } else {
      _task_wait = delay;
    }
  }
  ++_task_time;

  for (auto& proj : _projs) {
    proj.tick();
  }

  if (!_boss.hide) {
    _boss.tick();
  }

  _player.tick();

  std::erase_if(_projs, [](const auto& proj) {
    const auto pos = proj.transform.pos();
    const float extra = 10.f;
    return (
      pos.x < -extra || 
      pos.y < -extra || 
      pos.x > VIEWPORT.x+extra || 
      pos.y > VIEWPORT.y+extra
    );
  });

  ++_tick_count;
}

void context::render(double dt, [[maybe_unused]] double alpha) {
  _viewport.bind(render::win_size(), [this]() {
    render::clear_viewport();

    auto& boss = _boss;
    if (!boss.hide) {
      render::draw_sprite(boss.sprite(), boss.mat(), _viewport.proj(), _viewport.view());
    }

    auto& player = _player;
    render::draw_sprite(player.sprite(), player.mat(), _viewport.proj(), _viewport.view());

    for (auto& proj : _projs) {
      render::draw_sprite(proj.sprite(), proj.mat(), _viewport.proj(), _viewport.view());
    }
  });
  render::draw_background(dt);
  _viewport.draw(render::win_proj());
  ntf::transform2d text_transform;
  text_transform
    .set_pos(30.f, 100.f)
    .set_scale(.75f);
  std::string txt = fmt::format("danmaku: {}", _projs.size());
  render::draw_text(txt, color4{1.f}, text_transform.mat());
  auto cpos = _player.transform.pos();
  txt = fmt::format("cino pos: {} {}", cpos.x, cpos.y);
  text_transform.set_pos(30.f, 140.f);
  render::draw_text(txt, color4{1.f}, text_transform.mat());


  ++_frame_count;
}

projectile_view::projectile_view(std::size_t size) : _size(size) {
  std::list<stage::projectile> list;
  const auto atlas = res::get_atlas("effects").value();
  const auto entry = atlas->group_at(atlas->find_group("star_med").value())[1];
  const auto aspect = atlas->at(entry).aspect();

  ntf::transform2d transform;
  transform
    .set_pos((vec2)VIEWPORT*.5f)
    .set_scale(aspect*20.f);

  for (size_t i = 0; i < size; ++i) {
    list.emplace_back(stage::projectile::args{
      .transform = transform,
      .movement = stage::entity_movement_linear(cmplx{0.f}),
      .animator = stage::entity_animator_static(atlas, entry),
      .clean_flag = false,
    });
  }
  _begin = list.begin();
  _curr_ctx->projs().splice(_curr_ctx->projs().end(), list);
}

void projectile_view::for_each(sol::function f) {
  auto it = _begin;
  for (std::size_t i = 0; i < _size; ++i, ++it) {
    f(*it);
  }
}

} // namespace stage

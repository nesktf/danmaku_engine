#define SOL_ALL_SAFETIES_ON 1
#include <sol/sol.hpp>

#include "stage/stage.hpp"

#include "resources.hpp"
#include "render.hpp"

#include <shogle/core/allocator.hpp>

namespace {

struct {
  sol::state lua;
  sol::protected_function entrypoint;

  frames tick_count{0}, frame_count{0};
  frames task_time{0}, task_wait{0};

  std::list<stage::projectile> projs;
  stage::boss boss;
  stage::player player;

  render::stage_viewport viewport;

  render::viewport_event::subscription vp_sub;
} stg;

} // namespace

class projectile_view {
public:
  using iterator = stage::entity_list<stage::projectile>::iterator;

public:
  projectile_view(std::size_t size) : _size(size) {
    stage::entity_list<stage::projectile> list;

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
    stg.projs.splice(stg.projs.end(), list);
  }

  void for_each(sol::function f) {
    auto it = _begin;
    for (std::size_t i = 0; i < _size; ++i, ++it) {
      f(*it);
    }
  }

public:
  std::size_t size() const { return _size; }

private:
  iterator _begin;
  std::size_t _size;
};

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

  auto view_type = module.new_usertype<projectile_view>(
    "pview", sol::call_constructor, sol::constructors<projectile_view(std::size_t)>{},
    "size", &projectile_view::size,
    "for_each", &projectile_view::for_each
  );

  return module;
};

void stage::init() {
  auto fb_shader = res::get_shader("framebuffer");
  stg.viewport.init(VIEWPORT, VIEWPORT/2, (vec2)render::win_size()*.5f, fb_shader.value());
  stg.vp_sub = render::vp_subscribe([](size_t w, size_t h) {
    stg.viewport.update_pos(vec2{w, h}*.5f);
  });
}

void stage::destroy() {
  render::vp_unsuscribe(stg.vp_sub);
  stg.viewport.destroy();
}

static void prepare_player() {
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
  stg.player = stage::player{stage::player::args{
    .transform = transform,
    .atlas = atlas,
    .anim = anim,
    .base_speed = sp,
    .slow_speed = sp*.66f,
  }};
}

static void prepare_lua_env(sol::state_view lua) {
  lua["__GLOBAL"] = lua.create_table_with(
    "dt", DT,
    "ups", UPS,
    "ticks", 0
  );
  lua["package"]["path"] = ";res/script/?.lua";

  // Log functions
  lua.set_function("__LOG_ERROR", [](std::string msg) { ntf::log::error("{}", msg); });
  lua.set_function("__LOG_WARNING", [](std::string msg) { ntf::log::warning("{}", msg); });
  lua.set_function("__LOG_DEBUG", [](std::string msg) { ntf::log::debug("{}", msg); });
  lua.set_function("__LOG_INFO", [](std::string msg) { ntf::log::info("{}", msg); });
  lua.set_function("__LOG_VERBOSE", [](std::string msg) { ntf::log::verbose("{}", msg); });

  // Boss functions
  lua.set_function("__SPAWN_BOSS", [](float scale, float, sol::table p0, sol::table p1) {
    const auto atlas = res::get_atlas("effects").value();
    const auto entry = atlas->group_at(atlas->find_group("ball_solid").value())[0];
    const auto aspect = atlas->at(entry).aspect();

    cmplx first = cmplx{p0.get<float>("real"), p0.get<float>("imag")};
    cmplx last = cmplx{p1.get<float>("real"), p1.get<float>("imag")};

    ntf::transform2d transform;
    transform
      .set_pos(first)
      .set_scale(scale*aspect);

    stg.boss = stage::boss{stage::boss::args{
      .transform = transform,
      .movement = stage::entity_movement_towards(last, DT*cmplx{10.f,10.f}, cmplx{DT}, .8f),
      .animator = stage::entity_animator_static(atlas, entry),
    }};
  });

  lua.set_function("__MOVE_BOSS", [](sol::table pos) {
    cmplx new_pos = cmplx{pos.get<float>("real"), pos.get<float>("imag")};
    stg.boss.movement =
      stage::entity_movement_towards(new_pos, DT*cmplx{10.f,10.f}, cmplx{DT}, .8f);
  });

  lua.set_function("__BOSS_POS", []() -> sol::table {
    const auto pos = stg.boss.transform.pos();
    return stg.lua.create_table_with("real", pos.x, "imag", pos.y);
  });
  
  // Player functions
  lua.set_function("__PLAYER_POS", []() -> sol::table {
    const auto pos = stg.player.transform.pos();
    return stg.lua.create_table_with("x", pos.x, "y", pos.y);
  });

  // Projectile functions
  lua.set_function("__SPAWN_DANMAKU", [](sol::table dir, float speed, sol::table pos) {
    cmplx proj_dir = cmplx{dir.get<float>("real"), dir.get<float>("imag")};
    cmplx proj_pos = cmplx{pos.get<float>("real"), pos.get<float>("imag")};

    const auto atlas = res::get_atlas("effects").value();
    const auto entry = atlas->group_at(atlas->find_group("star_med").value())[3];
    const auto aspect = atlas->at(entry).aspect();

    ntf::transform2d transform;
    transform
      .set_pos(proj_pos)
      .set_scale(aspect*20.f);

    stg.projs.emplace_back(stage::projectile::args {
      .transform = transform,
      .movement = stage::entity_movement_linear(proj_dir*speed),
      .animator = stage::entity_animator_static(atlas, entry),
      .angular_speed = 2*M_PIf*DT,
    });
  });

  // Misc
  lua.set_function("__STAGE_TICKS", []() -> frames { return stg.tick_count; });
  lua.set_function("__VIEWPORT", []() -> sol::table {
    return stg.lua.create_table_with("x", VIEWPORT.x, "y", VIEWPORT.y);
  });


  lua.require("funny_lib", sol::c_call<decltype(&funny_lib), &funny_lib>);
}

static void reset_state() {
  stg.player = stage::player{};
  stg.boss = stage::boss{};
  stg.projs.clear();
  stg.tick_count = 0;
  stg.frame_count = 0;
  stg.task_time = 0;
  stg.task_wait = 0;
  stg.entrypoint = sol::protected_function{};
  stg.lua = sol::state{};
}

void stage::load(std::string_view stage_script) {
  auto& lua = stg.lua;

  reset_state();

  prepare_player();

  lua.open_libraries(
    sol::lib::base, sol::lib::coroutine,
    sol::lib::package, sol::lib::table,
    sol::lib::math, sol::lib::string
  );

  prepare_lua_env(lua);

  lua.script_file(stage_script.data());

  sol::protected_function init_task = lua["__INIT_TASK"];
  if (init_task) {
    init_task();
  } else {
    ntf::log::warning("[stage] Lua initial task not defined in script {}", stage_script);
  }

  stg.entrypoint = lua["__MAIN_TASK"];
  if (!stg.entrypoint) {
    throw ntf::error{"[stage] Lua main task not defined in script {}", stage_script};
  }
}

static void clean_projectiles() {
  const float extra = 10.f;
  std::erase_if(stg.projs, [&](const auto& proj) {
    const auto pos = proj.transform.pos();
    return (
      pos.x < -extra || 
      pos.y < -extra || 
      pos.x > VIEWPORT.x+extra || 
      pos.y > VIEWPORT.y+extra
    );
  });
}

void stage::tick() {
  // auto& lua = stg.lua;
  // sol::function on_tick = lua["__ON_TICK"];
  // on_tick();

  if (stg.task_time >= stg.task_wait) {
    stg.task_time = 0;

    sol::table task_yield = stg.entrypoint();
    const auto delay = task_yield.get<frame_delay>("wait_time");
    if (delay < 0) {
      ntf::log::debug("[stage] Lua main task returned");
    } else {
      stg.task_wait = delay;
    }
  }
  ++stg.task_time;

  for (auto& proj : stg.projs) {
    proj.tick();
  }

  if (!stg.boss.hide) {
    stg.boss.tick();
  }

  stg.player.tick();

  clean_projectiles();

  ++stg.tick_count;
}

void stage::render(double dt, [[maybe_unused]] double alpha) {
  stg.viewport.bind(render::win_size(), []() {
    render::clear_viewport();

    auto& boss = stg.boss;
    if (!boss.hide) {
      render::draw_sprite(boss.sprite(), boss.mat(), stg.viewport.proj(), stg.viewport.view());
    }

    auto& player = stg.player;
    render::draw_sprite(player.sprite(), player.mat(), stg.viewport.proj(), stg.viewport.view());

    for (auto& proj : stg.projs) {
      render::draw_sprite(proj.sprite(), proj.mat(), stg.viewport.proj(), stg.viewport.view());
    }
  });
  render::draw_background(dt);
  stg.viewport.draw(render::win_proj());
  ntf::transform2d text_transform;
  text_transform
    .set_pos(30.f, 100.f)
    .set_scale(.75f);
  std::string txt = fmt::format("danmaku: {}", stg.projs.size());
  render::draw_text(txt, color4{1.f}, text_transform.mat());
  auto cpos = stg.player.transform.pos();
  txt = fmt::format("cino pos: {} {}", cpos.x, cpos.y);
  text_transform.set_pos(30.f, 140.f);
  render::draw_text(txt, color4{1.f}, text_transform.mat());


  ++stg.frame_count;
}

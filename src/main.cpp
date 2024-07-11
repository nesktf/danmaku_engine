#include <shogle/shogle.hpp>
#include <shogle/core/registry.hpp>

// #include "state.hpp"
// #include "render.hpp"
// #include "player.hpp"

using namespace ntf;

static ntf::cleanup _ {[]{
  shogle::engine_destroy();
}};

// static game::state game_state{};
// game::state& game::global_state = game_state;
//
// static game::res game_res{};
// game::res& resources = game_res;
//
// auto filter = shogle::tex_filter::nearest;
// auto wrap = shogle::tex_wrap::repeat;
//
//
// static std::vector<uptr<game::entity>> entities;
//
// void game_init() {
//   game_res.spritesheets.emplace(std::make_pair("effects",
//     shogle::load_spritesheet("res/spritesheets/effects.json", filter, wrap)));
//   game_res.spritesheets.emplace(std::make_pair("chara",
//     shogle::load_spritesheet("res/spritesheets/chara.json", filter, wrap)));
//   game_res.fonts.emplace(std::make_pair("arial",
//     shogle::load_font("res/fonts/arial.ttf")));
//
//   game_state.cam = shogle::camera2d{shogle::engine_window_size()};
//   game::render_init();
//
//   auto& chara = game_res.spritesheets.at("chara");
//   auto& marisa_idle = chara["marisa_idle"];
//   entities.emplace_back(make_uptr<game::player>(&marisa_idle));
//
//   log::debug("[game] Init");
// }
//
// void game_render(double dt, double fdt, double alpha) {
//   shogle::render_clear(color3{0.2f});
//   for (auto& e : entities) {
//     e->on_render(dt, alpha);
//   }
// }
//
// void game_tick(double dt) {
//   for (auto& e : entities) {
//     e->on_tick(dt);
//   }
// }
//
// void game_update(double dt) {
//   for (auto& e : entities) {
//     e->on_update(dt);
//   }
// }
//
// void game_viewport_event(size_t w, size_t h) {
//   shogle::render_viewport(w, h);
//   game_state.cam.set_viewport(w, h).update();
// }
//
// void game_key_event(shogle::keycode kcode, shogle::keystate kstate) {
//   if (kcode == shogle::key_escape && kstate == shogle::press) {
//     shogle::engine_close_window();
//   }
//   for (auto& e : entities) {
//     e->on_input(kcode, kstate);
//   }
// }
//

using registry = shogle::registry<4096>;

struct say_dou_component : public registry::component<say_dou_component> {
  say_dou_component() = default;
  void tick(registry::entity entity, float dt) const {
    log::debug("DOU {}", entity.id());
  }
};


int main() {
  log::set_level(loglevel::verbose);

  shogle::engine_init(1024, 768, "test");

  shogle::engine_use_vsync(false);
  shogle::render_blending(true);

  registry reg;

  auto ent = reg.create();
  auto d = say_dou_component{};
  ent.add(say_dou_component{});

  auto ent2 = reg.create();
  ent2.add(say_dou_component{});

  auto on_render = [](double dt, double fixed_dt, double alpha) {
    shogle::render_clear(color3{0.2f});
  };

  auto on_tick = [&](double dt) {
    for (auto& [c, id] : say_dou_component::get_set()) {
      c.tick(registry::entity{id, &reg}, dt);
    }
  };

  shogle::engine_viewport_event([](size_t w, size_t h) {
  });

  shogle::engine_key_event([](shogle::keycode code, auto, shogle::keystate state, auto) {
  });

  shogle::engine_main_loop(60, on_render, on_tick);
}


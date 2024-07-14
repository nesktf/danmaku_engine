#include <shogle/shogle.hpp>

#include "render.hpp"
#include "state.hpp"
#include "enemy.hpp"

using namespace ntf;

static ntf::cleanup _ {[]{shogle::engine_destroy();}};

static game::game_state _state;
game::game_state& game::state = _state;

static game::res _resources;
game::res& game::resources = _resources;

auto filter = shogle::tex_filter::nearest;
auto wrap = shogle::tex_wrap::repeat;

static shogle::font* arial;

using namespace ntf::game;

void init() {
  game::render_init();
  state.camera = shogle::camera2d{shogle::engine_window_size()};

  resources.spritesheets.emplace("chara", 
      shogle::load_spritesheet("res/spritesheets/chara.json", filter, wrap));
  resources.spritesheets.emplace("effects",
      shogle::load_spritesheet("res/spritesheets/effects.json", filter, wrap));
  resources.spritesheets.emplace("enemies",
      shogle::load_spritesheet("res/spritesheets/enemies.json", filter, wrap));
  
  resources.fonts.emplace("arial",
      shogle::load_font("res/fonts/arial.ttf"));

  state.player.set_sprite(
      &resources.spritesheets.at("chara")["marisa_idle"], &resources.spritesheets.at("effects")["hitbox"]);

  arial = &resources.fonts.at("arial");

  auto viewport = shogle::engine_window_size();
  state.curr_level = make_uptr<test_level>();
  state.level_fbo = make_uptr<shogle::framebuffer>(viewport.x, viewport.y);

  log::debug("[game] Init done");
}

static cmplx vel(float t, cmplx dir) {
  return cmplx{100.0f, 100.0f*glm::cos(PI*t)}*dir;
}

void tick(double dt) {
  state.curr_level->tick(dt);
}

void render(double dt, double fdt, double alpha) {
  shogle::render_clear(color3{0.2f});
  state.curr_level->render();
  render_text(*arial, vec2{5.0f, 25.0f}, 0.5f, color4{1.0f}, "fps: {:.2f}", 1/dt);
}

void viewport_event(size_t w, size_t h) {
  shogle::render_viewport(w, h);
  state.camera.set_viewport(w, h).update();
}

void key_event(shogle::keycode code, shogle::keystate state) {
  if (code == shogle::key_escape && state == shogle::press) {
    shogle::engine_close_window();
  }
}

int main() {
  log::set_level(loglevel::verbose);
  shogle::engine_init(1024, 768, "test");
  shogle::engine_use_vsync(false);
  shogle::render_blending(true);

  init();
  shogle::engine_viewport_event(viewport_event);
  shogle::engine_key_event([](shogle::keycode code, auto, shogle::keystate state, auto) {
    key_event(code, state);
  });

  shogle::engine_main_loop(60, render, tick);
}


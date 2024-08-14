#include "global.hpp"
#include "render.hpp"
#include "resources.hpp"
#include "frontend.hpp"

#include <shogle/engine.hpp>

#include <shogle/render/framebuffer.hpp>

#include <shogle/scene/transform.hpp>
#include <shogle/scene/camera.hpp>

static struct {
  mat4 proj;
  ivec2 size;
} window;

static struct {
  mat4 proj;
  mat4 view;
  vec2 cam_pos{0.0f};
  ntf::transform2d transform;
  ntf::framebuffer viewport;
} stage_viewport;

static void update_window_mat(size_t w, size_t h) {
  float znear = -10.0f;
  float zfar = 1.0f;
  window.proj = glm::ortho(
    0.0f, (float)w,
    (float)h, 0.0f,
    znear, zfar
  );
  window.size = ivec2{w, h};
}

static void update_viewport_mat([[maybe_unused]] size_t w, [[maybe_unused]] size_t h) {
  float znear = -10.0f;
  float zfar = 1.0f;
  stage_viewport.proj = glm::ortho(
    0.0f, (float)VIEWPORT.x,
    (float)VIEWPORT.y, 0.0f,
    znear, zfar
  );
  stage_viewport.cam_pos = (vec2)VIEWPORT*0.5f;
  stage_viewport.view = 
    ntf::view2d((vec2)VIEWPORT*0.5f, stage_viewport.cam_pos, vec2{1.0f}, 0.0f);
}

static void update_viewport(size_t w, size_t h) {
  ntf::render_viewport(w, h);
  update_window_mat(w, h);
  stage_viewport.transform.set_pos(vec2{w,h}*0.5f);
}

void render::destroy() {
  ntf::engine_destroy();
}

void render::init() {
  // Load OpenGL and things
  ntf::engine_init(WIN_SIZE.x, WIN_SIZE.y, "test");
  ntf::engine_viewport_event(update_viewport);

  ntf::engine_use_vsync(false);
  ntf::render_blending(true);
}

void render::post_init() {
  // Prepare shaders and things
  auto vp = ntf::engine_window_size();
  update_window_mat(vp.x, vp.y);
  update_viewport_mat(vp.y, vp.y);

  stage_viewport.viewport = ntf::framebuffer{VIEWPORT.x, VIEWPORT.y};
  stage_viewport.transform.set_pos((vec2)vp*0.5f) // At the middle of the window
    .set_scale(VIEWPORT);

  // TODO: cache shader uniforms
}

static void draw_sprite(res::sprite_id sprite, ntf::transform2d& transform, const color4& color) {
  const auto& shader = res::shader_at("sprite");
  const auto& sheet = res::spritesheet_at(sprite.sheet);
  const auto& sprite_data = sheet[sprite.index];

  const auto sprite_sampler = 0;

  ntf::render_use_shader(shader);

  ntf::render_set_uniform(shader, "proj", stage_viewport.proj);
  ntf::render_set_uniform(shader, "view", stage_viewport.view);
  ntf::render_set_uniform(shader, "model", transform.mat());

  ntf::render_set_uniform(shader, "offset", sprite_data.offset);
  ntf::render_set_uniform(shader, "sprite_color", color);

  ntf::render_set_uniform(shader, "sprite_sampler", (int)sprite_sampler);
  ntf::render_bind_sampler(sheet.tex(), (size_t)sprite_sampler);

  ntf::render_draw_quad();
}

static void draw_player() {
  auto& player = global::state().stage.player;
  draw_sprite(player.spr, player.transform(), color4{1.0f});
}

static void draw_boss() {
  auto& boss = global::state().stage.boss;

  if (!boss.ready()) {
    return;
  }

  draw_sprite(boss.spr(), boss.transform(), color4{1.0f});
}

static void draw_stage() {
  for (auto& bullet : global::state().stage.projectiles) {
    const auto& sprite = bullet.sprite;
    auto& transform = bullet.transform;
    draw_sprite(sprite, transform, color4{1.0f});
  }
}

static void draw_gui() {
  const auto& vp_shader = res::shader_at("framebuffer");
  const auto framebuffer_sampler = 0;

  ntf::render_use_shader(vp_shader);

  ntf::render_set_uniform(vp_shader, "proj", window.proj);
  ntf::render_set_uniform(vp_shader, "view", mat4{1.0f});
  ntf::render_set_uniform(vp_shader, "model", stage_viewport.transform.mat());

  ntf::render_set_uniform(vp_shader, "fb_sampler", (int)framebuffer_sampler);
  ntf::render_bind_sampler(stage_viewport.viewport.tex(), (size_t)framebuffer_sampler);

  ntf::render_draw_quad();
}

static void render_gameplay() {
  const auto& wsz = window.size;
  stage_viewport.viewport.bind(wsz.x, wsz.y, [](){
    ntf::render_clear(color3{0.3f});
    draw_boss();
    draw_player();
    draw_stage();
  });
  draw_gui();
}

static void render_frontend(double) {
  const auto& font = res::font_at(res::font_index("arial"));
  const auto& font_shader = res::shader_at(res::shader_index("font"));

  auto& menu = frontend::state().entry();
  draw_sprite(menu.background, menu.back_transform, color4{1.0f});

  for (size_t i = 0; i < menu.entries.size(); ++i) {
    const auto focused_index = menu.focused;
    color4 col{1.0f};
    if (i == focused_index) {
      col = color4{1.0f, 0.0f, 0.0f, 1.0f};
    }

    ntf::transform2d font_transform;
    const vec2 pos {100.0f,i*50.0f + 200.0f};
    font_transform.set_pos(pos);

    ntf::render_use_shader(font_shader);
    
    ntf::render_set_uniform(font_shader, "proj", window.proj);
    ntf::render_set_uniform(font_shader, "model", font_transform.mat());
    ntf::render_set_uniform(font_shader, "text_color", col);

    const auto shader_sampler = 0;
    ntf::render_set_uniform(font_shader, "tex", (int)shader_sampler);
    ntf::render_draw_text(font, vec2{0.0f, 0.0f}, 1.0f, menu.entries[i].text);
  }
}

static void render_font(res::font_id font_id, std::string_view text) {
  const auto& font = res::font_at(font_id);

  const auto shad_id = res::shader_index("font");
  const auto& shader = res::shader_at(shad_id);
  auto text_world_width = [&]() -> float{
    float width{};
    for (auto c = text.begin(); c != text.end(); ++c) {
      auto [tex, ch] = font._glyph_tex.at(*c);
      width += (ch.advance >> 6);
    }
    return width;
  };

}

static void render_loading() {

}

void render::draw([[maybe_unused]] double dt, [[maybe_unused]] double alpha) {
  ntf::render_clear(color3{0.2f});

  switch (global::state().current_state) {
    case global::states::loading: {
      render_loading();
      break;
    }
    case global::states::frontend: {
      render_frontend(dt);
      break;
    }
    case global::states::gameplay: {
      render_gameplay();
      break;
    }
  }
}


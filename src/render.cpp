#include "render.hpp"
#include "resources.hpp"
#include "stage.hpp"

static struct {
  mat4 proj;
  ivec2 size;
} window;

static struct {
  mat4 proj;
  mat4 view;
  vec2 cam_pos{0.0f};
  transform2d transform;
  framebuffer viewport;
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

static void update_viewport_mat(size_t w, size_t h) {
  float znear = -10.0f;
  float zfar = 1.0f;
  stage_viewport.proj = glm::ortho(
    0.0f, (float)VIEWPORT.x,
    (float)VIEWPORT.y, 0.0f,
    znear, zfar
  );
  stage_viewport.cam_pos = (vec2)VIEWPORT*0.5f;
  stage_viewport.view = 
    ntf::shogle::view2d((vec2)VIEWPORT*0.5f, stage_viewport.cam_pos, vec2{1.0f}, 0.0f);
}

static void update_viewport(size_t w, size_t h) {
  ntf::shogle::render_viewport(w, h);
  update_window_mat(w, h);
  stage_viewport.transform.set_pos(vec2{w,h}*0.5f);
}

void render::destroy() {
  ntf::shogle::engine_destroy();
}

void render::init() {
  // Load OpenGL and things
  ntf::shogle::engine_init(WIN_SIZE.x, WIN_SIZE.y, "test");
  ntf::shogle::engine_viewport_event(update_viewport);

  ntf::shogle::engine_use_vsync(false);
  ntf::shogle::render_blending(true);
}

void render::post_init() {
  // Prepare shaders and things
  auto vp = ntf::shogle::engine_window_size();
  update_window_mat(vp.x, vp.y);
  update_viewport_mat(vp.y, vp.y);

  stage_viewport.viewport = framebuffer{VIEWPORT.x, VIEWPORT.y};
  stage_viewport.transform.set_pos((vec2)vp*0.5f) // At the middle of the window
    .set_scale(VIEWPORT);

  // TODO: cache shader uniforms
}

static void draw_sprite(const ntf::shogle::sprite& sprite, transform2d& transform, const color4& color) {
  const auto& shader = res::shader("sprite");
  const auto sprite_sampler = 0;

  ntf::shogle::render_use_shader(shader);

  ntf::shogle::render_set_uniform(shader, "proj", stage_viewport.proj);
  ntf::shogle::render_set_uniform(shader, "view", stage_viewport.view);
  ntf::shogle::render_set_uniform(shader, "model", transform.mat());

  ntf::shogle::render_set_uniform(shader, "offset", sprite.texture_offset);
  ntf::shogle::render_set_uniform(shader, "sprite_color", color);

  ntf::shogle::render_set_uniform(shader, "sprite_sampler", (int)sprite_sampler);
  ntf::shogle::render_bind_sampler(*sprite.texture, (size_t)sprite_sampler);

  ntf::shogle::render_draw_quad();
}

static void draw_player() {
  auto& player = stage::player();
  draw_sprite(player.spr, player.transform(), color4{1.0f});
}

static void draw_boss() {
  auto& boss = stage::boss();

  if (!boss.ready()) {
    return;
  }

  draw_sprite(boss.spr(), boss.transform(), color4{1.0f});
}

static void draw_stage() {
  for (auto& bullet : stage::projectiles()) {
    const auto& sprite = bullet.sprite;
    auto& transform = bullet.transform;
    draw_sprite(sprite, transform, color4{1.0f});
  }
}

static void draw_gui() {
  const auto& vp_shader = res::shader("framebuffer");
  const auto framebuffer_sampler = 0;

  ntf::shogle::render_use_shader(vp_shader);

  ntf::shogle::render_set_uniform(vp_shader, "proj", window.proj);
  ntf::shogle::render_set_uniform(vp_shader, "view", mat4{1.0f});
  ntf::shogle::render_set_uniform(vp_shader, "model", stage_viewport.transform.mat());

  ntf::shogle::render_set_uniform(vp_shader, "fb_sampler", (int)framebuffer_sampler);
  ntf::shogle::render_bind_sampler(stage_viewport.viewport.tex(), (size_t)framebuffer_sampler);

  ntf::shogle::render_draw_quad();
}

void render::draw(double dt, double alpha) {
  ntf::shogle::render_clear(color3{0.2f});

  const auto& wsz = window.size;
  stage_viewport.viewport.bind(wsz.x, wsz.y, [](){
    ntf::shogle::render_clear(color3{0.3f});
    draw_boss();
    draw_player();
    draw_stage();
  });
  draw_gui();
}

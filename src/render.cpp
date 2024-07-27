#include "render.hpp"
#include "global.hpp"

#include <shogle/engine.hpp>

#include <shogle/render/framebuffer.hpp>

#include <shogle/res/shader.hpp>

#include <shogle/scene/transform.hpp>
#include <shogle/scene/camera.hpp>

using namespace ntf;

static struct {
  shogle::sprite_shader sprite_shader;
  shogle::font_shader font_shader;
  shogle::framebuffer_shader viewport_shader;

  glm::mat4 viewport_proj;
  glm::mat4 viewport_view;
  vec2 viewport_cam_pos{0.0f};
  shogle::framebuffer viewport_fb;
  shogle::transform2d viewport_transform;

  glm::mat4 window_proj;
  ivec2 window_size;
} render_state;

static void update_window_mat(size_t w, size_t h) {
  float znear = -10.0f;
  float zfar = 1.0f;
  render_state.window_proj = glm::ortho(
    0.0f, (float)w,
    (float)h, 0.0f,
    znear, zfar
  );
  render_state.window_size = ivec2{w, h};
}

static void update_viewport_mat(size_t w, size_t h) {
  float znear = -10.0f;
  float zfar = 1.0f;
  render_state.viewport_proj = glm::ortho(
    0.0f, (float)VIEWPORT.x,
    (float)VIEWPORT.y, 0.0f,
    znear, zfar
  );
  render_state.viewport_view = 
    shogle::view2d((vec2)VIEWPORT*0.5f, render_state.viewport_cam_pos, vec2{1.0f}, 0.0f);
}

void ntf::render_init() {
  shogle::engine_use_vsync(false);
  shogle::render_blending(true);

  auto vp = shogle::engine_window_size();
  update_window_mat(vp.x, vp.y);
  update_viewport_mat(vp.y, vp.y);

  render_state.sprite_shader.compile();
  render_state.font_shader.compile();
  render_state.viewport_shader.compile();

  render_state.viewport_fb = shogle::framebuffer{VIEWPORT.x, VIEWPORT.y};
  render_state.viewport_transform.set_pos((vec2)vp*0.5f) // At the middle of the window
    .set_scale(VIEWPORT);
}

static void render_sprite(const shogle::sprite& sprite, shogle::transform2d& transform,
                          size_t index, const color4& color) {
  render_state.sprite_shader.enable()
    .set_proj(render_state.viewport_proj)
    .set_view(render_state.viewport_view)
    .set_transform(transform.mat())
    .set_color(color)
    .bind_sprite(sprite, index);
  shogle::render_draw_quad();
}

template<typename Fun>
static void render_viewport(Fun&& fun) {
  auto& wsz = render_state.window_size;
  render_state.viewport_fb.bind(wsz.x, wsz.y, std::forward<Fun>(fun));
  render_state.viewport_shader.enable()
    .set_proj(render_state.window_proj)
    .set_view(mat4{1.0f})
    .set_transform(render_state.viewport_transform.mat())
    .bind_framebuffer(render_state.viewport_fb);
  shogle::render_draw_quad();
}

void ntf::render_new_frame(double dt, double alpha) {
  shogle::render_clear(color3{0.2f});
  render_viewport([&]() {
    shogle::render_clear(color3{0.3f});
    for (auto& bullet : global.projectiles) {
      auto& transform = bullet.transf();
      const auto& sprite = bullet.spr();
      render_sprite(sprite, transform, 0, color4{1.0f});
    }
  });
}

void ntf::render_update_viewport(size_t w, size_t h) {
  shogle::render_viewport(w, h);
  update_window_mat(w, h);
  render_state.viewport_transform.set_pos(vec2{w,h}*0.5f);
}

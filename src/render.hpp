#pragma once

#include "resources.hpp"

#include <shogle/core/event.hpp>
#include <shogle/scene/transform.hpp>
#include <shogle/render/gl/framebuffer.hpp>

namespace render {

using window = glfw::window<renderer>;

using uniform = renderer::shader_uniform;

using viewport_event = ntf::event<size_t, size_t>;

class ui_renderer {
public:
  void init(ivec2 win_size, res::shader shader);
  void tick(double dt);
  void draw(const renderer::texture2d& tex, const mat4& win_proj);

private:
  float _back_time{0.f};
  ntf::transform2d _ui_root;
  res::shader _shader;
  uniform _proj_u, _model_u;
  uniform _time_u, _sampler_u;
};

class stage_viewport {
public:
  void init(ivec2 vp_size, ivec2 center, vec2 pos, res::shader shader);
  void destroy();
  void draw(const mat4& win_proj);

  template<typename Fun>
  void bind(ivec2 win_size, Fun&& f);

  void update_viewport(ivec2 vp_size, ivec2 center);
  void update_pos(vec2 pos);

public:
  const mat4& view() { return _view; }
  const mat4& proj() { return _proj; }

private:
  mat4 _proj, _view;
  renderer::framebuffer _viewport;
  ntf::transform2d _transform;
  vec2 _cam_center;

  res::shader _shader;
  uniform _proj_u, _view_u, _model_u, _sampler_u;
};

template<typename Fun>
void stage_viewport::bind(ivec2 win_size, Fun&& f) {
  _viewport.bind(win_size, std::forward<Fun>(f));
}


void init(window& win);
void post_init(window& win);
void destroy();

void draw_sprite(res::sprite sprite, const mat4& mod, const mat4& proj, const mat4& view);
void draw_background(double dt);
void draw_frontend(double dt);

void clear_viewport();
ivec2 win_size();
const mat4& win_proj();

viewport_event::subscription vp_subscribe(viewport_event::callback callback);
void vp_unsuscribe(viewport_event::subscription sub);

} // namespace render

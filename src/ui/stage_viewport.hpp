#pragma once

#include "ui/widget.hpp"

#include "resources.hpp"
#include "render.hpp"

#include <shogle/render/framebuffer.hpp>

#include <shogle/scene/camera.hpp>

namespace ui {

class stage_viewport : public widget {
public:
  stage_viewport(ivec2 vp_size, ivec2 center, vec2 pos, std::unique_ptr<render::shader_renderer> fb_shader) : 
    _fb_shader(std::move(fb_shader)) {
    update_viewport(vp_size, center);
    update_pos(pos);
  }

public:
  void draw(const render::window_data& window) override {
    _fb_shader->render(_fb.tex(), _transform, window.proj, window.view);
  }

  template<typename StageRenderer>
  void bind(ivec2 win_size, StageRenderer&& renderer) {
    _fb.bind(win_size.x, win_size.y, std::forward<StageRenderer>(renderer));
  }

  void update_viewport(ivec2 vp_size, ivec2 center) {
    _cam_center = (vec2)center;
    _fb = ntf::framebuffer{(size_t)vp_size.x, (size_t)vp_size.y};
    _stage_proj = glm::ortho(0.f, (float)vp_size.x, (float)vp_size.y, 0.f, -10.f, 1.f);
    _stage_view = ntf::view2d((vec2)_fb.size()*.5f, _cam_center, vec2{1.f}, 0.f);
    _transform.set_scale((vec2)vp_size);
  }

  void update_pos(vec2 pos) {
    _transform.set_pos(pos);
  }

public:
  const mat4& proj() { return _stage_proj; }
  const mat4& view() { return _stage_view; }

private:
  ntf::framebuffer _fb;
  std::unique_ptr<render::shader_renderer> _fb_shader;
  mat4 _stage_proj, _stage_view;
  vec2 _cam_center;
};

} // namespace ui

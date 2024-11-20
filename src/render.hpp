#pragma once

#include "okuu.hpp"

#include "resources.hpp"

#include "ui/widget.hpp"

namespace okuu {

class ui_renderer {
public:
  void init(ivec2 win_size, okuu::resource<okuu::shader> shader);
  void tick(double dt);
  void draw(const renderer::texture2d& tex, const mat4& win_proj);

private:
  float _back_time{0.f};
  ntf::transform2d _ui_root;
  okuu::resource<okuu::shader> _shader;
  uniform _proj_u, _model_u;
  uniform _time_u, _sampler_u;
};


class stage_viewport {
public:
  void init(ivec2 vp_size, ivec2 center, vec2 pos, okuu::resource<okuu::shader> shader);
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
  okuu::framebuffer _viewport;
  ntf::transform2d _transform;
  vec2 _cam_center;

  okuu::resource<okuu::shader> _shader;
  uniform _proj_u, _view_u, _model_u, _sampler_u;
};

class sprite_renderer {
public:
  void init(okuu::resource<okuu::shader> shader);

  void draw(okuu::sprite, const color4& color,
            const mat4& model, const mat4& proj, const mat4& view) const;

private:
  okuu::resource<okuu::shader> _shader;
  okuu::uniform _proj_u, _view_u, _model_u;
  okuu::uniform _offset_u, _color_u, _sampler_u;
};


class render_context {
public:
  void post_init(okuu::resource_manager& res);
  void draw_sprite(okuu:sprite sprite, ntf::transform2d& transform, ntf::camera2d& camera) const;
  void draw_text(okuu::resource<okuu::font> font, color4 color, ntf::transform2d& transform, 
                 std::string_view text) const;
  void clear_viewport();

private:
  render_context(okuu::window& window);

private:
  void _on_viewport_event(std::size_t w, std::size_t h);

private:
  okuu::window& _window;
  okuu::ui_renderer _ui;
  okuu::sprite_renderer _sprite;

  std::stack<ui::widget*> _widgets;
  // ntf::camera2d _cam2d;

private:
  friend class okuu::context;

public:
  NTF_DECLARE_NO_MOVE_NO_COPY(render_context);
};

template<typename Fun>
void okuu::stage_viewport::bind(ivec2 win_size, Fun&& f) {
  _viewport.bind(win_size, std::forward<Fun>(f));
}

// void init(window& win);
// void post_init(window& win);
// void destroy();
//
// void draw_sprite(res::sprite sprite, const mat4& mod, const mat4& proj, const mat4& view);
// void draw_background(double dt);
// void draw_frontend(double dt);
// void draw_text(std::string_view text, color4 color, const mat4& mod);
//
// void clear_viewport();
// ivec2 win_size();
// const mat4& win_proj();
//
// viewport_event::subscription vp_subscribe(viewport_event::callback callback);
// void vp_unsuscribe(viewport_event::subscription sub);

} // namespace okuu

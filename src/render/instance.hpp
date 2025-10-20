#pragma once

#include "../event.hpp"
#include "./common.hpp"

namespace okuu::render {

struct shader_data {
  shogle::vertex_shader vert_sprite_generic;
  shogle::fragment_shader frag_sprite_generic;
};

struct okuu_render_ctx {
  okuu_render_ctx(shogle::window&& win_, shogle::context&& ctx_, shader_data&& shaders_,
                  shogle::quad_mesh&& quad_, shogle::pipeline&& stage_pip_,
                  shogle::texture2d&& base_fb_tex_, shogle::framebuffer&& base_fb_,
                  shogle::pipeline&& back_pip_);

  shogle::window win;
  shogle::context ctx;
  shader_data shaders;
  event_handler<u32, u32> viewport_event;
  shogle::quad_mesh quad;
  shogle::pipeline stage_pip;
  shogle::texture2d base_fb_tex;
  shogle::framebuffer base_fb;
  shogle::pipeline back_pip;
  shogle::texture2d missing_tex;
};

extern ntf::nullable<okuu_render_ctx> g_renderer;

} // namespace okuu::render

// #include <shogle/core/event.hpp>
// #include <shogle/render/gl/framebuffer.hpp>
// #include <shogle/scene/transform.hpp>
//
// namespace render {
//
// using window = glfw::window<renderer>;
//
// using uniform = renderer::shader_uniform;
//
// using viewport_event = ntf::event<size_t, size_t>;
//
// class ui_renderer {
// public:
//   void init(ivec2 win_size, res::shader shader);
//   void tick(double dt);
//   void draw(const renderer::texture2d &tex, const mat4 &win_proj);
//
// private:
//   float _back_time{0.f};
//   ntf::transform2d _ui_root;
//   res::shader _shader;
//   uniform _proj_u, _model_u;
//   uniform _time_u, _sampler_u;
// };
//
// class stage_viewport {
// public:
//   void init(ivec2 vp_size, ivec2 center, vec2 pos, res::shader shader);
//   void destroy();
//   void draw(const mat4 &win_proj);
//
//   template <typename Fun> void bind(ivec2 win_size, Fun &&f);
//
//   void update_viewport(ivec2 vp_size, ivec2 center);
//   void update_pos(vec2 pos);
//
// public:
//   const mat4 &view() { return _view; }
//   const mat4 &proj() { return _proj; }
//
// private:
//   mat4 _proj, _view;
//   renderer::framebuffer _viewport;
//   ntf::transform2d _transform;
//   vec2 _cam_center;
//
//   res::shader _shader;
//   uniform _proj_u, _view_u, _model_u, _sampler_u;
// };
//
// template <typename Fun> void stage_viewport::bind(ivec2 win_size, Fun &&f) {
//   _viewport.bind(win_size, std::forward<Fun>(f));
// }
//
// void init(window &win);
// void post_init(window &win);
// void destroy();
//
// void draw_sprite(res::sprite sprite, const mat4 &mod, const mat4 &proj,
//                  const mat4 &view);
// void draw_background(double dt);
// void draw_frontend(double dt);
// void draw_text(std::string_view text, color4 color, const mat4 &mod);
//
// void clear_viewport();
// ivec2 win_size();
// const mat4 &win_proj();
//
// viewport_event::subscription vp_subscribe(viewport_event::callback callback);
// void vp_unsuscribe(viewport_event::subscription sub);
//
// } // namespace render

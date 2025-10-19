#pragma once

#include <chimatools/chimatools.h>
#include <shogle/shogle.hpp>

namespace okuu::render {

using namespace ntf::numdefs;
using shogle::mat4;
using shogle::uvec2;
using shogle::vec2;
using shogle::vec3;

enum class viewport_res {
  x480p = 0, // by 560 (6:7), 640 (4:3), 360 (16:9)
  x600p,
  x720p,
  x900p,
  x1024p,
  x1080p,
};

template<typename T>
using expect = ntf::expected<T, std::string>;

struct singleton_handle {
  singleton_handle() noexcept = default;
  ~singleton_handle() noexcept;
};

[[nodiscard]] singleton_handle init();

shogle::window& window();

shogle::context_view shogle_ctx();

void render_back(float t);

expect<shogle::texture2d> upload_spritesheet(const chima_spritesheet& sheet);

expect<shogle::pipeline> create_pipeline();

class stage_viewport {
private:
  struct stage_uniforms {
    mat4 proj;
    mat4 view;
  };

  static constexpr f32 DEFAULT_ASPECT = 6.f / 7.f;
  static constexpr uvec2 DEFAULT_SIZE{600, 700};

public:
  stage_viewport(shogle::texture2d&& fb_tex, shogle::framebuffer&& fb, shogle::pipeline&& pip,
                 shogle::shader_storage_buffer&& ssbo, u32 xpos, u32 ypos);

public:
  static stage_viewport create(u32 width, u32 height, u32 xpos, u32 ypos);

public:
  void render();

  const shogle::framebuffer& framebuffer() const { return _fb; }

  const shogle::shader_storage_buffer& ssbo() const { return _ssbo; }

  const shogle::shader_binding binds() const {
    return {
      .buffer = _ssbo,
      .binding = 1,
      .size = _ssbo.size(),
      .offset = 0,
    };
  }

private:
  shogle::texture2d _fb_tex;
  shogle::framebuffer _fb;
  shogle::pipeline _pip;
  shogle::shader_storage_buffer _ssbo;

  // The stage viewport works with screen space coordinates
  // Each pixel should map 1:1 to the window viewport
  u32 _xpos, _ypos;
  stage_uniforms _unif;
};

struct sprite_command {
  ntf::weak_cptr<shogle::pipeline> pipeline;
  ntf::weak_cptr<shogle::texture2d> texture;
  ntf::nullable<shogle::shader_binding> ssbo_bind;
};

void render_sprites(stage_viewport& stage, std::vector<sprite_command>& cmds);

struct ui_element {
  virtual ~ui_element() = default;
  virtual void render_commands(std::vector<sprite_command>& cmds);
};

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

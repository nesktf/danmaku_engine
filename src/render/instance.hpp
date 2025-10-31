#pragma once

#include "../event.hpp"
#include "./stage.hpp"

namespace okuu::render {

struct shader_data {
  shogle::vertex_shader vert_sprite_generic;
  shogle::fragment_shader frag_sprite_generic;
};

class sprite_renderer {
public:
  static constexpr size_t MAX_RENDER_AUX_TEX = 8u;
  static constexpr size_t SHADER_SAMPLER_COUNT = MAX_SPRITE_AUX_TEX + 1;

  struct sprite_shader_data {
    mat4 transform;
    i32 samplers[SHADER_SAMPLER_COUNT];
    i32 ticks;
    f32 uv_lin_x;
    f32 uv_lin_y;
    f32 uv_con_x;
    f32 uv_con_y;
  };

public:
  sprite_renderer(u32 instances, shogle::shader_storage_buffer&& ssbo, shogle::quad_mesh&& quad);

public:
  shogle::quad_mesh& quad() { return _quad; }

public:
  void enqueue_sprite(const sprite_render_data& sprite_data);
  void render();

private:
  u32 _max_instances;
  u32 _sprite_instance_num;
  shogle::shader_storage_buffer _buffer;
  shogle::quad_mesh _quad;
  std::array<shogle::texture_binding, MAX_RENDER_AUX_TEX> _texes;
  u32 _active_texes;
};

struct okuu_render_ctx {
  okuu_render_ctx(shogle::window&& win_, shogle::context&& ctx_, shader_data&& shaders_,
                  sprite_renderer&& sprite_ren_, shogle::pipeline&& stage_pip_,
                  shogle::texture2d&& base_fb_tex_, shogle::framebuffer&& base_fb_,
                  shogle::pipeline&& back_pip_);

  shogle::window win;
  shogle::context ctx;
  shader_data shaders;
  event_handler<u32, u32> viewport_event;
  sprite_renderer sprite_ren;
  shogle::pipeline stage_pip;
  shogle::texture2d base_fb_tex;
  shogle::framebuffer base_fb;
  shogle::pipeline back_pip;
  shogle::texture2d missing_tex;
  std::vector<ntf::inplace_function<void(shogle::context_view)>> render_cbs;
};

extern ntf::nullable<okuu_render_ctx> g_renderer;

} // namespace okuu::render

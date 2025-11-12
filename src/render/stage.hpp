#pragma once

#include "./common.hpp"

namespace okuu::render {

class stage_viewport {
private:
  static constexpr f32 DEFAULT_ASPECT = 6.f / 7.f;
  static constexpr uvec2 DEFAULT_SIZE{600, 700};

public:
  stage_viewport(u32 width, u32 height, u32 xpos, u32 ypos, shogle::texture2d&& fb_tex,
                 shogle::framebuffer&& fb);

public:
  static expect<stage_viewport> create(u32 width, u32 height, u32 xpos, u32 ypos);

public:
  shogle::framebuffer_view framebuffer() const { return {_fb}; }

  shogle::texture_binding tex_binds(u32 sampler) const;

  std::pair<u32, u32> extent() const;

  std::pair<u32, u32> pos() const;

  mat4 transform() const;
  mat4 proj() const;
  mat4 view() const;

private:
  shogle::texture2d _fb_tex;
  shogle::framebuffer _fb;
  u32 _width, _height;
  u32 _xpos, _ypos;
};

struct sprite_uvs {
  f32 x_lin, x_con;
  f32 y_lin, y_con;
};

class stage_renderer {
private:
  enum SHADER_BIND {
    SHADER_VERTEX_BIND = 0,
    SHADER_FRAGMENT_BIND,
    SHADER_BIND_COUNT,
  };

public:
  static constexpr size_t MAX_SHADER_SAMPLERS = 8u;
  static constexpr u32 DEFAULT_STAGE_INSTANCES = 1024u;

  struct sprite_vertex_data {
    mat4 transform;
    mat4 view;
    mat4 proj;
    f32 uv_scale_x;
    f32 uv_scale_y;
    f32 uv_offset_x;
    f32 uv_offset_y;
  };

  struct sprite_fragment_data {
    f32 color_r;
    f32 color_g;
    f32 color_b;
    f32 color_a;
    i32 sampler;
    i32 ticks;
  };

  struct sprite_render_data {
    mat4 transform;
    shogle::texture2d_view texture;
    u32 ticks;
    sprite_uvs uvs;
    color4 color;
  };

public:
  stage_renderer(u32 instances, stage_viewport&& viewport,
                 shogle::shader_storage_buffer&& sprite_vert_buffer,
                 shogle::shader_storage_buffer&& sprite_frag_buffer);

public:
  static expect<stage_renderer> create(u32 instances = DEFAULT_STAGE_INSTANCES);

public:
  stage_viewport& viewport() { return _viewport; }

  ntf::cspan<shogle::texture_binding> tex_binds() const;
  ntf::cspan<shogle::shader_binding> shader_binds() const;

public:
  u32 sprite_instances() const { return _sprite_instances; }

  void reset_instances();
  void enqueue_sprite(const sprite_render_data& sprite_data);

private:
  stage_viewport _viewport;
  shogle::shader_storage_buffer _sprite_vert_buffer;
  shogle::shader_storage_buffer _sprite_frag_buffer;
  std::array<shogle::texture_binding, MAX_SHADER_SAMPLERS> _tex_binds;
  std::array<shogle::shader_binding, SHADER_BIND_COUNT> _sprite_buffer_binds;
  u32 _active_texes;
  u32 _max_instances;
  u32 _sprite_instances;
};

void render_stage(stage_renderer& stage);
void render_viewport(stage_viewport& viewport, shogle::framebuffer_view target);

} // namespace okuu::render

#pragma once

#include "./common.hpp"

namespace okuu::render {

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

  std::pair<u32, u32> extent() const {
    auto ext = _fb_tex.extent();
    return std::make_pair(ext.x, ext.y);
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

static constexpr size_t MAX_SPRITE_AUX_TEX = 3u;

struct sprite_render_data {
  ntf::weak_cptr<stage_viewport> viewport;
  shogle::texture2d_view texture;
  std::array<shogle::texture2d_view, MAX_SPRITE_AUX_TEX> extra_texs;
  u32 ticks;
  vec2 position;
  vec2 scale;
  f32 rotation;
  vec2 uv_lin;
  vec2 uv_con;
};

void render_sprite(const sprite_render_data& sprite);

} // namespace okuu::render

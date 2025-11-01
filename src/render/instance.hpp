#pragma once

#include "../util/event.hpp"
#include "./stage.hpp"

namespace okuu::render {

struct shader_data {
  shogle::vertex_shader vert_sprite_generic;
  shogle::fragment_shader frag_sprite_generic;
};

struct okuu_render_ctx {
  okuu_render_ctx(shogle::window&& win_, shogle::context&& ctx_, shader_data&& shaders_,
                  stage_renderer&& sprite_ren_, shogle::pipeline&& stage_pip_,
                  shogle::texture2d&& base_fb_tex_, shogle::framebuffer&& base_fb_,
                  shogle::pipeline&& back_pip_);

  shogle::window win;
  shogle::context ctx;
  shader_data shaders;
  util::event_handler<u32, u32> viewport_event;
  shogle::quad_mesh quad;
  shogle::pipeline sprite_pipeline;
  shogle::pipeline stage_pip;
  shogle::texture2d base_fb_tex;
  shogle::framebuffer base_fb;
  shogle::pipeline back_pip;
  shogle::texture2d missing_tex;
};

extern ntf::nullable<okuu_render_ctx> g_renderer;

} // namespace okuu::render

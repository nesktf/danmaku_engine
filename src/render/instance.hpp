#pragma once

#include "../util/event.hpp"
#include "./stage.hpp"

namespace okuu::render {

struct base_pipelines {
  shogle::pipeline viewport;
  shogle::pipeline sprite;
  shogle::pipeline back;
};

expect<base_pipelines> init_pipelines(shogle::context_view ctx);

expect<shogle::pipeline> make_pip(shogle::context_view ctx, shogle::vertex_shader_view vert,
                                  shogle::fragment_shader_view frag,
                                  ntf::cspan<shogle::attribute_binding> attribs);

struct okuu_render_ctx {
public:
  okuu_render_ctx(shogle::window&& win_, shogle::context&& ctx_, shogle::quad_mesh&& quad_,
                  shogle::texture2d&& missing_tex_, shogle::texture2d&& fb_tex_,
                  shogle::framebuffer&& fb_, base_pipelines&& pips_);

public:
  shogle::window win;
  shogle::context ctx;

public:
  shogle::quad_mesh quad;
  shogle::texture2d missing_tex;
  shogle::texture2d fb_tex;
  shogle::framebuffer fb;
  base_pipelines pips;

public:
  util::event_handler<u32, u32> viewport_event;
};

extern ntf::nullable<okuu_render_ctx> g_renderer;

} // namespace okuu::render

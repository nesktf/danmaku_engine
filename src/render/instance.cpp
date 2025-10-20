#include "./instance.hpp"
#include "./shader_src.hpp"
#include <ntfstl/logger.hpp>

namespace okuu::render {

namespace {

template<u32 tex_extent>
[[maybe_unused]] static constexpr auto missing_albedo_bitmap = [] {
  std::array<u8, 4u * tex_extent * tex_extent> out;
  const u8 pixels[]{
    0x00, 0x00, 0x00, 0xFF, // black
    0xFE, 0x00, 0xFE, 0xFF, // pink
    0x00, 0x00, 0x00, 0xFF, // black again
  };

  for (u32 i = 0; i < tex_extent; ++i) {
    const u8* start = i % 2 == 0 ? &pixels[0] : &pixels[4]; // Start row with a different color
    u32 pos = 0;
    for (u32 j = 0; j < tex_extent; ++j) {
      pos = (pos + 4) % 8;
      for (u32 k = 0; k < 4; ++k) {
        out[(4 * i * tex_extent) + (4 * j) + k] = start[pos + k];
      }
    }
  }

  return out;
}();

template<u32 tex_extent = 16u>
static shogle::render_expect<shogle::texture2d> make_missing_albedo(shogle::context_view ctx) {
  const shogle::image_data image{
    .bitmap = missing_albedo_bitmap<tex_extent>.data(),
    .format = shogle::image_format::rgba8u,
    .alignment = 4u,
    .extent = {tex_extent, tex_extent, 1},
    .offset = {0, 0, 0},
    .layer = 0u,
    .level = 0u,
  };
  const shogle::texture_data data{
    .images = {image},
    .generate_mipmaps = false,
  };
  const shogle::typed_texture_desc desc{
    .format = shogle::image_format::rgba8u,
    .sampler = shogle::texture_sampler::nearest,
    .addressing = shogle::texture_addressing::repeat,
    .extent = {tex_extent, tex_extent, 1},
    .layers = 1u,
    .levels = 1u,
    .data = data,
  };
  return shogle::texture2d::create(ctx, desc);
}

std::string shogle_to_str(shogle::render_error&& err) {
  return err.what();
}

const shogle::blend_opts blending{
  .mode = shogle::blend_mode::add,
  .src_factor = shogle::blend_factor::src_alpha,
  .dst_factor = shogle::blend_factor::inv_src_alpha,
  .color = {0.f, 0.f, 0.f, 0.f},
};

const shogle::depth_test_opts depth_test{
  .func = shogle::test_func::less,
  .near_bound = 0.f,
  .far_bound = 1.f,
};

} // namespace

ntf::nullable<okuu_render_ctx> g_renderer;

okuu_render_ctx::okuu_render_ctx(shogle::window&& win_, shogle::context&& ctx_,
                                 shader_data&& shaders_, shogle::quad_mesh&& quad_,
                                 shogle::pipeline&& stage_pip_, shogle::texture2d&& base_fb_tex_,
                                 shogle::framebuffer&& base_fb_, shogle::pipeline&& back_pip_) :
    win{std::move(win_)},
    ctx{std::move(ctx_)}, shaders{std::move(shaders_)}, quad{std::move(quad_)},
    stage_pip{std::move(stage_pip_)}, base_fb_tex{std::move(base_fb_tex_)},
    base_fb{std::move(base_fb_)}, back_pip{std::move(back_pip_)},
    missing_tex{make_missing_albedo<4>(ctx).value()} {}

[[nodiscard]] singleton_handle init() {
  const u32 win_width = 1280;
  const u32 win_height = 720;

  NTF_ASSERT(!g_renderer.has_value());
  const shogle::win_x11_params x11{
    .class_name = "okuu_engine",
    .instance_name = "okuu_engine",
  };
  const shogle::win_gl_params win_gl{
    .ver_major = 4,
    .ver_minor = 6,
    .swap_interval = 1,
    .fb_msaa_level = 8,
    .fb_buffer = shogle::fbo_buffer::depth24u_stencil8u,
    .fb_use_alpha = false,
  };
  const shogle::win_params win_params{
    .width = win_width,
    .height = win_height,
    .title = "test",
    .attrib = shogle::win_attrib::decorate | shogle::win_attrib::resizable,
    .renderer_api = shogle::context_api::opengl,
    .platform_params = &x11,
    .renderer_params = &win_gl,
  };
  auto win = shogle::window::create(win_params).value();

  const auto fb_clear = shogle::clear_flag::color_depth;
  const shogle::color4 fb_color{.3f, .3f, .3f, 1.f};
  const auto vp = shogle::uvec4{0, 0, win.fb_size()};
  const auto gl_params = shogle::window::make_gl_params(win);

  const shogle::context_params ctx_params{
    .ctx_params = &gl_params,
    .ctx_api = shogle::context_api::opengl,
    .fb_viewport = vp,
    .fb_clear_flags = fb_clear,
    .fb_clear_color = fb_color,
    .alloc = nullptr,
  };
  auto ctx = shogle::context::create(ctx_params).value();

  auto sprite_vert_shader = shogle::vertex_shader::create(ctx, {vert_sprite}).value();
  auto sprite_frag_shader = shogle::fragment_shader::create(ctx, {frag_sprite}).value();
  auto quad = shogle::quad_mesh::create(ctx).value();

  auto vert_fbo_shader = shogle::vertex_shader::create(ctx, {vert_fbo}).value();
  auto frag_fbo_shader = shogle::fragment_shader::create(ctx, {frag_fbo}).value();

  const auto attribs = shogle::pnt_vertex::aos_binding();
  const shogle::shader_t stages_fb[] = {vert_fbo_shader.get(), frag_fbo_shader.get()};
  const shogle::pipeline_desc pip_desc{
    .attributes = {attribs.data(), attribs.size()},
    .stages = stages_fb,
    .primitive = shogle::primitive_mode::triangles,
    .poly_mode = shogle::polygon_mode::fill,
    .poly_width = 1.f,
    .tests =
      {
        .stencil_test = nullptr,
        .depth_test = depth_test,
        .scissor_test = nullptr,
        .face_culling = nullptr,
        .blending = blending,
      },
  };

  auto pip_vp = shogle::pipeline::create(ctx, pip_desc).value();

  const shogle::typed_texture_desc fb_tex_desc{
    .format = shogle::image_format::rgba8u,
    .sampler = shogle::texture_sampler::nearest,
    .addressing = shogle::texture_addressing::repeat,
    .extent = {1280, 720, 1},
    .layers = 1u,
    .levels = 1u,
    .data = nullptr,
  };
  auto fb_tex = shogle::texture2d::create(ctx, fb_tex_desc).value();

  const shogle::fbo_image fb_img{
    .texture = fb_tex,
    .layer = 0,
    .level = 0,
  };
  const shogle::fbo_image_desc fb_desc{.extent = {1280, 720},
                                       .viewport = {0, 0, 1280, 720},
                                       .clear_color{1.f, .0f, .0f, 1.f},
                                       .clear_flags = shogle::clear_flag::color_depth,
                                       .test_buffer = shogle::fbo_buffer::depth24u_stencil8u,
                                       .images = {fb_img}};
  auto fb = shogle::framebuffer::create(ctx, fb_desc).value();

  auto frag_back_shader = shogle::fragment_shader::create(ctx, {frag_back}).value();

  const shogle::shader_t stages_back[] = {vert_fbo_shader.get(), frag_back_shader.get()};
  const shogle::pipeline_desc back_pip_desc{
    .attributes = {attribs.data(), attribs.size()},
    .stages = stages_back,
    .primitive = shogle::primitive_mode::triangles,
    .poly_mode = shogle::polygon_mode::fill,
    .poly_width = 1.f,
    .tests =
      {
        .stencil_test = nullptr,
        .depth_test = depth_test,
        .scissor_test = nullptr,
        .face_culling = nullptr,
        .blending = blending,
      },
  };
  auto pip_back = shogle::pipeline::create(ctx, back_pip_desc).value();

  g_renderer.emplace(std::move(win), std::move(ctx),
                     shader_data{
                       .vert_sprite_generic = std::move(sprite_vert_shader),
                       .frag_sprite_generic = std::move(sprite_frag_shader),
                     },
                     std::move(quad), std::move(pip_vp), std::move(fb_tex), std::move(fb),
                     std::move(pip_back));
  NTF_ASSERT(g_renderer.has_value());
  g_renderer->win.set_viewport_callback([](auto&, uvec2 vp) {
    shogle::framebuffer::get_default(g_renderer->ctx).viewport({0.f, 0.f, vp.x, vp.y});
    g_renderer->viewport_event.fire(vp.x, vp.y);
  });
  return {};
}

singleton_handle::~singleton_handle() noexcept {
  NTF_ASSERT(g_renderer.has_value());
  g_renderer.reset();
}

shogle::window& window() {
  NTF_ASSERT(g_renderer.has_value());
  return g_renderer->win;
}

shogle::context_view shogle_ctx() {
  NTF_ASSERT(g_renderer.has_value());
  return g_renderer->ctx;
}

expect<shogle::pipeline> create_pipeline() {
  NTF_ASSERT(g_renderer.has_value());

  const auto& shaders = g_renderer->shaders;
  const auto attribs = shogle::pnt_vertex::aos_binding();
  const shogle::shader_t stages[] = {shaders.vert_sprite_generic.get(),
                                     shaders.frag_sprite_generic.get()};
  return shogle::pipeline::create(g_renderer->ctx,
                                  {
                                    .attributes = {attribs.data(), attribs.size()},
                                    .stages = stages,
                                    .primitive = shogle::primitive_mode::triangles,
                                    .poly_mode = shogle::polygon_mode::fill,
                                    .poly_width = 1.f,
                                    .tests =
                                      {
                                        .stencil_test = nullptr,
                                        .depth_test = depth_test,
                                        .scissor_test = nullptr,
                                        .face_culling = nullptr,
                                        .blending = blending,
                                      },
                                  })
    .transform_error(shogle_to_str);
}

void render_back(float t) {
  NTF_ASSERT(g_renderer.has_value());
  auto fb = shogle::framebuffer::get_default(g_renderer->ctx);
  auto& pip = g_renderer->back_pip;

  auto loc_proj = pip.uniform_location("proj").value();
  auto loc_model = pip.uniform_location("model").value();
  auto loc_sampler = pip.uniform_location("tex").value();
  auto loc_time = pip.uniform_location("time").value();

  const auto proj = glm::ortho(0.f, 1280.f, 720.f, 0.f, -10.f, 1.f);
  auto transf = shogle::transform2d<float>{}.scale(1280 * 2, 720 * 2);

  const mat4 model = transf.world();
  shogle::uniform_const unifs[] = {
    shogle::format_uniform_const(loc_model, model),
    shogle::format_uniform_const(loc_proj, proj),
    shogle::format_uniform_const(loc_time, t),
    shogle::format_uniform_const(loc_sampler, 0),
  };

  const shogle::texture_binding tbind{
    .texture = g_renderer->missing_tex,
    .sampler = 0,
  };

  g_renderer->ctx.submit_render_command({
    .target = fb,
    .pipeline = pip,
    .buffers = g_renderer->quad.bindings(),
    .textures = {tbind},
    .consts = unifs,
    .opts =
      {
        .vertex_count = 6,
        .vertex_offset = 0,
        .index_offset = 0,
        .instances = 0,
      },
    .sort_group = 0,
    .render_callback = {},
  });
}

} // namespace okuu::render

// #include "global.hpp"
// #include "render.hpp"
// #include "ui/frontend.hpp"
//
// #include <shogle/engine.hpp>
//
// #include <shogle/scene/camera.hpp>
//
// namespace {
//
// class sprite_renderer {
// public:
//   void init(res::shader shader);
//
//   void draw(res::sprite sprite, const color4& color, const mat4& model, const mat4& proj,
//             const mat4& view) const;
//
// private:
//   res::shader _shader;
//   render::uniform _proj_u, _view_u, _model_u;
//   render::uniform _offset_u, _color_u, _sampler_u;
// };
//
// void sprite_renderer::init(res::shader shader) {
//   _shader = shader;
//
//   _shader->uniform_location(_proj_u, "proj");
//   _shader->uniform_location(_view_u, "view");
//   _shader->uniform_location(_model_u, "model");
//   _shader->uniform_location(_offset_u, "offset");
//   _shader->uniform_location(_color_u, "sprite_color");
//   _shader->uniform_location(_sampler_u, "sprite_sampler");
// }
//
// void sprite_renderer::draw(res::sprite sprite, const color4& color, const mat4& model,
//                            const mat4& proj, const mat4& view) const {
//   const auto [atlas, index] = sprite;
//   const auto sprite_sampler = 0;
//
//   _shader->use();
//   _shader->set_uniform(_proj_u, proj);
//   _shader->set_uniform(_view_u, view);
//   _shader->set_uniform(_model_u, model);
//   _shader->set_uniform(_offset_u, atlas->at(index).offset);
//   _shader->set_uniform(_color_u, color);
//
//   _shader->set_uniform(_sampler_u, (int)sprite_sampler);
//   atlas->texture().bind_sampler((size_t)sprite_sampler);
//
//   renderer::draw_quad();
// }
//
// struct {
//   render::ui_renderer ui;
//   // render::stage_viewport stage;
//   sprite_renderer sprite;
//
//   mat4 win_proj;
//   ivec2 win_size;
//
//   render::viewport_event vp_event;
// } r;
//
// } // namespace
//
// namespace render {
//
// void destroy() {
//   r.vp_event.clear();
// }
//
// void init(ntf::glfw::window<renderer>& window) {
//   // Load OpenGL and things
//   renderer::set_blending(true);
//
//   window.set_viewport_event([](size_t w, size_t h) {
//     renderer::set_viewport(w, h);
//     r.win_size = ivec2{w, h};
//     r.win_proj = glm::ortho(0.f, (float)w, (float)h, 0.f, -10.f, 1.f);
//
//     r.vp_event.fire(w, h);
//   });
// }
//
// void post_init(ntf::glfw::window<renderer>& win) {
//   // Prepare shaders and things
//   auto sprite_shader = res::get_shader("sprite");
//   auto front_shader = res::get_shader("frontend");
//
//   auto vp = win.size();
//
//   r.win_size = vp;
//   r.win_proj = glm::ortho(0.f, (float)vp.x, (float)vp.y, 0.f, -10.f, 1.f);
//   r.ui.init(vp, front_shader.value());
//   r.sprite.init(sprite_shader.value());
// }
//
// void draw_background(double dt) {
//   const auto& back = res::texture{0}.get();
//   r.ui.tick(dt);
//   r.ui.draw(back, r.win_proj);
// }
//
// void draw_frontend(double dt) {
//   // TODO: Move this thing to a widget class
//   render::draw_background(dt);
//
//   const auto& font = res::get_font("arial")->get();
//   const auto& font_shader = res::get_shader("font")->get();
//
//   auto& menu = frontend::instance().entry();
//   render::draw_sprite(menu.background, menu.back_transform.mat(), r.win_proj, mat4{1.f});
//
//   for (size_t i = 0; i < menu.entries.size(); ++i) {
//     const auto focused_index = menu.focused;
//     color4 col{1.0f};
//     if (i == focused_index) {
//       col = color4{1.0f, 0.0f, 0.0f, 1.0f};
//     }
//
//     ntf::transform2d font_transform;
//     const vec2 pos{100.0f, i * 50.0f + 200.0f};
//     font_transform.pos(pos);
//
//     const auto shader_sampler = 0;
//
//     font_shader.use();
//     font_shader.set_uniform("proj", r.win_proj);
//     font_shader.set_uniform("model", font_transform.mat());
//     font_shader.set_uniform("text_color", col);
//     font_shader.set_uniform("tex", (int)shader_sampler);
//     font.draw_text(vec2{0.f, 0.f}, 1.f, menu.entries[i].text);
//   }
// }
//
// void draw_text(std::string_view text, color4 color, const mat4& mod) {
//   const auto font = res::get_font("arial").value();
//   const auto font_shader = res::get_shader("font").value();
//   const auto shader_sampler = 0;
//
//   font_shader->use();
//   font_shader->set_uniform("proj", r.win_proj);
//   font_shader->set_uniform("model", mod);
//   font_shader->set_uniform("text_color", color);
//   font_shader->set_uniform("tex", (int)shader_sampler);
//   font->draw_text(vec2{0.f}, 1.f, text);
// }
//
// void clear_viewport() {
//   renderer::clear_viewport(color3{.3f});
// }
//
// void draw_sprite(res::sprite sprite, const mat4& mod, const mat4& proj, const mat4& view) {
//   r.sprite.draw(sprite, color4{1.f}, mod, proj, view);
// }
//
// ivec2 win_size() {
//   return r.win_size;
// }
//
// const mat4& win_proj() {
//   return r.win_proj;
// }
//
// viewport_event::subscription vp_subscribe(viewport_event::callback callback) {
//   return r.vp_event.subscribe(callback);
// }
//
// void vp_unsuscribe(viewport_event::subscription sub) {
//   r.vp_event.unsubscribe(sub);
// }
//
// void ui_renderer::init(ivec2 win_size, res::shader shader) {
//   _shader = shader;
//
//   _shader->uniform_location(_proj_u, "proj");
//   _shader->uniform_location(_model_u, "model");
//   _shader->uniform_location(_time_u, "time");
//   _shader->uniform_location(_sampler_u, "tex");
//
//   _ui_root.pos((vec2)win_size * .5f).scale(win_size);
// }
//
// void ui_renderer::tick(double dt) {
//   _back_time += dt;
// }
//
// void ui_renderer::draw(const renderer::texture2d& tex, const mat4& win_proj) {
//   const auto sampler = 0;
//
//   _shader->use();
//   _shader->set_uniform(_proj_u, win_proj);
//   _shader->set_uniform(_model_u, _ui_root.mat());
//   _shader->set_uniform(_time_u, _back_time);
//
//   _shader->set_uniform(_sampler_u, (int)sampler);
//   tex.bind_sampler((size_t)sampler);
//
//   renderer::draw_quad();
// }
//
// void stage_viewport::init(ivec2 vp_size, ivec2 center, vec2 pos, res::shader shader) {
//   _shader = shader;
//   shader->uniform_location(_proj_u, "proj");
//   shader->uniform_location(_view_u, "view");
//   shader->uniform_location(_model_u, "model");
//   shader->uniform_location(_sampler_u, "fb_sampler");
//
//   update_viewport(vp_size, center);
//   update_pos(pos);
// }
//
// void stage_viewport::destroy() {
//   _viewport = renderer::framebuffer{};
// }
//
// void stage_viewport::draw(const mat4& win_proj) {
//   assert(_viewport.valid());
//   const auto sampler = 0;
//
//   _shader->use();
//
//   _shader->set_uniform(_proj_u, win_proj);
//   _shader->set_uniform(_view_u, mat4{1.f});
//   _shader->set_uniform(_model_u, _transform.mat());
//
//   _shader->set_uniform(_sampler_u, (int)sampler);
//   _viewport.tex().bind_sampler((size_t)sampler);
//
//   renderer::draw_quad();
// }
//
// void stage_viewport::update_viewport(ivec2 vp_size, ivec2 center) {
//   _cam_center = center;
//   _viewport = renderer::framebuffer{vp_size};
//   _proj = glm::ortho(0.f, (float)vp_size.x, (float)vp_size.y, 0.f, -10.f, 1.f);
//   _view = ntf::camera2d::build_view((vec2)_viewport.size() * .5f, _cam_center, vec2{1.f}, 0.f);
//   _transform.scale((vec2)vp_size);
// }
//
// void stage_viewport::update_pos(vec2 pos) {
//   _transform.pos(pos);
// }
//
// } // namespace render

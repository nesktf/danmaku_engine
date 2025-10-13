#include "./render.hpp"

namespace okuu {

namespace {

constexpr std::string_view vert_sprite = R"glsl(
#version 460 core

layout (location = 0) in vec3 att_coords;
layout (location = 1) in vec3 att_normals;
layout (location = 2) in vec2 att_texcoords;
out vec2 tex_coord;

uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_proj;

uniform vec4 u_offset;

void main() {
  tex_coord.x = att_texcoords.x*u_offset.x + u_offset.z;
  tex_coord.y = att_texcoords.y*u_offset.y + u_offset.w;

  gl_Position = u_proj * u_view * u_model * vec4(att_coords, 1.0f);
}

)glsl";

constexpr std::string_view frag_sprite = R"glsl(
#version 460 core

in vec2 tex_coord;
out vec4 frag_color;

uniform sampler2D u_sprite_sampler;
uniform vec4 u_sprite_color;

void main() {
  vec4 out_color = u_sprite_color * texture(u_sprite_sampler, tex_coord);

  if (out_color.a < 0.1) {
    discard;
  }

  frag_color = out_color;
}

)glsl";

struct shader_data {
  shogle::vertex_shader vert_sprite_generic;
  shogle::fragment_shader frag_sprite_generic;
};

struct okuu_render_ctx {
  okuu_render_ctx(shogle::window&& win_, shogle::context&& ctx_, shader_data&& shaders_) :
      win{std::move(win_)}, ctx{std::move(ctx_)}, shaders{std::move(shaders_)} {}

  shogle::window win;
  shogle::context ctx;
  shader_data shaders;
};

ntf::nullable<okuu_render_ctx> g_renderer;

} // namespace

void r_init() {
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
  auto win = shogle::window::create(
                 {
                     .width = win_width,
                     .height = win_height,
                     .title = "okuu engine",
                     .attrib = shogle::win_attrib::decorate | shogle::win_attrib::resizable,
                     .renderer_api = shogle::context_api::opengl,
                     .platform_params = &x11,
                     .renderer_params = &win_gl,
                 })
                 .value();

  const auto fb_clear = shogle::clear_flag::color_depth;
  const shogle::color4 fb_color{.3f, .3f, .3f, 1.f};
  const auto vp = shogle::uvec4{0, 0, win.fb_size()};
  const auto gl_params = shogle::window::make_gl_params(win);
  auto ctx = shogle::context::create({
                                         .ctx_params = &gl_params,
                                         .ctx_api = shogle::context_api::opengl,
                                         .fb_viewport = vp,
                                         .fb_clear_flags = fb_clear,
                                         .fb_clear_color = fb_color,
                                         .alloc = nullptr,
                                     })
                 .value();

  auto sprite_vert_shader = shogle::vertex_shader::create(ctx, {vert_sprite}).value();
  auto sprite_frag_shader = shogle::fragment_shader::create(ctx, {frag_sprite}).value();

  g_renderer.emplace(std::move(win), std::move(ctx),
                     shader_data{
                         .vert_sprite_generic = std::move(sprite_vert_shader),
                         .frag_sprite_generic = std::move(sprite_frag_shader),
                     });
}

void r_destroy() {
  NTF_ASSERT(g_renderer.has_value());
  g_renderer.reset();
}

shogle::window& r_get_window() {
  NTF_ASSERT(g_renderer.has_value());
  return g_renderer->win;
}

r_expect<shogle::texture2d> r_upload_spritesheet(const chima_spritesheet& sheet) {
  NTF_ASSERT(g_renderer.has_value());

  const auto& atlas = sheet.atlas;
  NTF_ASSERT(atlas.data);
  NTF_ASSERT(atlas.depth == chima_image_depth::CHIMA_DEPTH_8U);
  NTF_ASSERT(atlas.channels == 4u);

  const shogle::image_data image{
      .bitmap = atlas.data,
      .format = shogle::image_format::rgba8u,
      .alignment = 4u,
      .extent = {atlas.width, atlas.height, 1},
      .offset = {0, 0, 0},
      .layer = 0u,
      .level = 0u,
  };
  const shogle::texture_data data{
      .images = {image},
      .generate_mipmaps = false,
  };
  auto tex = shogle::texture2d::create(g_renderer->ctx,
                                       {
                                           .format = shogle::image_format::rgba8u,
                                           .sampler = shogle::texture_sampler::nearest,
                                           .addressing = shogle::texture_addressing::repeat,
                                           .extent = {atlas.width, atlas.height, 1},
                                           .layers = 1u,
                                           .levels = 1u,
                                           .data = data,
                                       });
  if (!tex) {
    return {ntf::unexpect, fmt::format("Failed to create texture, {}", tex.error().what())};
  }

  return {ntf::in_place, std::move(*tex)};
}

} // namespace okuu

#include "global.hpp"
#include "render.hpp"
#include "ui/frontend.hpp"

#include <shogle/engine.hpp>

#include <shogle/scene/camera.hpp>

namespace {

class sprite_renderer {
public:
  void init(res::shader shader);

  void draw(res::sprite sprite, const color4& color, const mat4& model, const mat4& proj,
            const mat4& view) const;

private:
  res::shader _shader;
  render::uniform _proj_u, _view_u, _model_u;
  render::uniform _offset_u, _color_u, _sampler_u;
};

void sprite_renderer::init(res::shader shader) {
  _shader = shader;

  _shader->uniform_location(_proj_u, "proj");
  _shader->uniform_location(_view_u, "view");
  _shader->uniform_location(_model_u, "model");
  _shader->uniform_location(_offset_u, "offset");
  _shader->uniform_location(_color_u, "sprite_color");
  _shader->uniform_location(_sampler_u, "sprite_sampler");
}

void sprite_renderer::draw(res::sprite sprite, const color4& color, const mat4& model,
                           const mat4& proj, const mat4& view) const {
  const auto [atlas, index] = sprite;
  const auto sprite_sampler = 0;

  _shader->use();
  _shader->set_uniform(_proj_u, proj);
  _shader->set_uniform(_view_u, view);
  _shader->set_uniform(_model_u, model);
  _shader->set_uniform(_offset_u, atlas->at(index).offset);
  _shader->set_uniform(_color_u, color);

  _shader->set_uniform(_sampler_u, (int)sprite_sampler);
  atlas->texture().bind_sampler((size_t)sprite_sampler);

  renderer::draw_quad();
}

struct {
  render::ui_renderer ui;
  // render::stage_viewport stage;
  sprite_renderer sprite;

  mat4 win_proj;
  ivec2 win_size;

  render::viewport_event vp_event;
} r;

} // namespace

namespace render {

void destroy() {
  r.vp_event.clear();
}

void init(ntf::glfw::window<renderer>& window) {
  // Load OpenGL and things
  renderer::set_blending(true);

  window.set_viewport_event([](size_t w, size_t h) {
    renderer::set_viewport(w, h);
    r.win_size = ivec2{w, h};
    r.win_proj = glm::ortho(0.f, (float)w, (float)h, 0.f, -10.f, 1.f);

    r.vp_event.fire(w, h);
  });
}

void post_init(ntf::glfw::window<renderer>& win) {
  // Prepare shaders and things
  auto sprite_shader = res::get_shader("sprite");
  auto front_shader = res::get_shader("frontend");

  auto vp = win.size();

  r.win_size = vp;
  r.win_proj = glm::ortho(0.f, (float)vp.x, (float)vp.y, 0.f, -10.f, 1.f);
  r.ui.init(vp, front_shader.value());
  r.sprite.init(sprite_shader.value());
}

void draw_background(double dt) {
  const auto& back = res::texture{0}.get();
  r.ui.tick(dt);
  r.ui.draw(back, r.win_proj);
}

void draw_frontend(double dt) {
  // TODO: Move this thing to a widget class
  render::draw_background(dt);

  const auto& font = res::get_font("arial")->get();
  const auto& font_shader = res::get_shader("font")->get();

  auto& menu = frontend::instance().entry();
  render::draw_sprite(menu.background, menu.back_transform.mat(), r.win_proj, mat4{1.f});

  for (size_t i = 0; i < menu.entries.size(); ++i) {
    const auto focused_index = menu.focused;
    color4 col{1.0f};
    if (i == focused_index) {
      col = color4{1.0f, 0.0f, 0.0f, 1.0f};
    }

    ntf::transform2d font_transform;
    const vec2 pos{100.0f, i * 50.0f + 200.0f};
    font_transform.pos(pos);

    const auto shader_sampler = 0;

    font_shader.use();
    font_shader.set_uniform("proj", r.win_proj);
    font_shader.set_uniform("model", font_transform.mat());
    font_shader.set_uniform("text_color", col);
    font_shader.set_uniform("tex", (int)shader_sampler);
    font.draw_text(vec2{0.f, 0.f}, 1.f, menu.entries[i].text);
  }
}

void draw_text(std::string_view text, color4 color, const mat4& mod) {
  const auto font = res::get_font("arial").value();
  const auto font_shader = res::get_shader("font").value();
  const auto shader_sampler = 0;

  font_shader->use();
  font_shader->set_uniform("proj", r.win_proj);
  font_shader->set_uniform("model", mod);
  font_shader->set_uniform("text_color", color);
  font_shader->set_uniform("tex", (int)shader_sampler);
  font->draw_text(vec2{0.f}, 1.f, text);
}

void clear_viewport() {
  renderer::clear_viewport(color3{.3f});
}

void draw_sprite(res::sprite sprite, const mat4& mod, const mat4& proj, const mat4& view) {
  r.sprite.draw(sprite, color4{1.f}, mod, proj, view);
}

ivec2 win_size() {
  return r.win_size;
}

const mat4& win_proj() {
  return r.win_proj;
}

viewport_event::subscription vp_subscribe(viewport_event::callback callback) {
  return r.vp_event.subscribe(callback);
}

void vp_unsuscribe(viewport_event::subscription sub) {
  r.vp_event.unsubscribe(sub);
}

void ui_renderer::init(ivec2 win_size, res::shader shader) {
  _shader = shader;

  _shader->uniform_location(_proj_u, "proj");
  _shader->uniform_location(_model_u, "model");
  _shader->uniform_location(_time_u, "time");
  _shader->uniform_location(_sampler_u, "tex");

  _ui_root.pos((vec2)win_size * .5f).scale(win_size);
}

void ui_renderer::tick(double dt) {
  _back_time += dt;
}

void ui_renderer::draw(const renderer::texture2d& tex, const mat4& win_proj) {
  const auto sampler = 0;

  _shader->use();
  _shader->set_uniform(_proj_u, win_proj);
  _shader->set_uniform(_model_u, _ui_root.mat());
  _shader->set_uniform(_time_u, _back_time);

  _shader->set_uniform(_sampler_u, (int)sampler);
  tex.bind_sampler((size_t)sampler);

  renderer::draw_quad();
}

void stage_viewport::init(ivec2 vp_size, ivec2 center, vec2 pos, res::shader shader) {
  _shader = shader;
  shader->uniform_location(_proj_u, "proj");
  shader->uniform_location(_view_u, "view");
  shader->uniform_location(_model_u, "model");
  shader->uniform_location(_sampler_u, "fb_sampler");

  update_viewport(vp_size, center);
  update_pos(pos);
}

void stage_viewport::destroy() {
  _viewport = renderer::framebuffer{};
}

void stage_viewport::draw(const mat4& win_proj) {
  assert(_viewport.valid());
  const auto sampler = 0;

  _shader->use();

  _shader->set_uniform(_proj_u, win_proj);
  _shader->set_uniform(_view_u, mat4{1.f});
  _shader->set_uniform(_model_u, _transform.mat());

  _shader->set_uniform(_sampler_u, (int)sampler);
  _viewport.tex().bind_sampler((size_t)sampler);

  renderer::draw_quad();
}

void stage_viewport::update_viewport(ivec2 vp_size, ivec2 center) {
  _cam_center = center;
  _viewport = renderer::framebuffer{vp_size};
  _proj = glm::ortho(0.f, (float)vp_size.x, (float)vp_size.y, 0.f, -10.f, 1.f);
  _view = ntf::camera2d::build_view((vec2)_viewport.size() * .5f, _cam_center, vec2{1.f}, 0.f);
  _transform.scale((vec2)vp_size);
}

void stage_viewport::update_pos(vec2 pos) {
  _transform.pos(pos);
}

} // namespace render

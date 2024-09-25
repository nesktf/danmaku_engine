#include "global.hpp"
#include "render.hpp"
#include "ui/frontend.hpp"

#include <shogle/engine.hpp>

#include <shogle/scene/camera.hpp>

namespace {

class sprite_renderer {
public:
  void init(res::shader shader);

  void draw(res::sprite sprite, const color4& color,
            const mat4& model, const mat4& proj, const mat4& view) const;

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

void sprite_renderer::draw(res::sprite sprite, const color4& color,
                         const mat4& model, const mat4& proj, const mat4& view) const {
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
    const vec2 pos {100.0f,i*50.0f + 200.0f};
    font_transform.set_pos(pos);

    const auto shader_sampler = 0;

    font_shader.use();
    font_shader.set_uniform("proj", r.win_proj);
    font_shader.set_uniform("model", font_transform.mat());
    font_shader.set_uniform("text_color", col);
    font_shader.set_uniform("tex", (int)shader_sampler);
    font.draw_text(vec2{0.f, 0.f}, 1.f, menu.entries[i].text);
  }
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

  _ui_root.set_pos((vec2)win_size*.5f).set_scale(win_size);
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
  _view = ntf::view2d((vec2)_viewport.size()*.5f, _cam_center, vec2{1.f}, 0.f);
  _transform.set_scale((vec2)vp_size);
}

void stage_viewport::update_pos(vec2 pos) {
  _transform.set_pos(pos);
}

} // namespace render


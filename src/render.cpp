#include "stage/state.hpp"

#include "global.hpp"
#include "render.hpp"
#include "ui/frontend.hpp"

#include <shogle/engine.hpp>

#include <shogle/render/gl/framebuffer.hpp>

#include <shogle/scene/camera.hpp>

namespace {

class window_viewport {
public:
  window_viewport() = default;
  window_viewport(ivec2 win) {
    update(win);
  }

public:
  void update(ivec2 win_size) {
    proj = glm::ortho(0.f, (float)win_size.x, (float)win_size.y, 0.f, -10.f, 1.f);
    size = win_size;
  }

public:
  mat4 proj{};
  ivec2 size{};
};

class stage_render {
public:
  stage_render() = default;
  stage_render(ivec2 vp_size, ivec2 center, vec2 pos, res::shader shader) : _shader(shader) {
    update_viewport(vp_size, center);
    update_pos(pos);
  }
  
public:
  void update_viewport(ivec2 vp_size, ivec2 center) {
    _cam_center = (vec2)center;
    _viewport = renderer::framebuffer{vp_size};
    _proj = glm::ortho(0.f, (float)vp_size.x, (float)vp_size.y, 0.f, -10.f, 1.f);
    _view = ntf::view2d((vec2)_viewport.size()*.5f, _cam_center, vec2{1.f}, 0.f);
    _transform.set_scale((vec2)vp_size);
  }

  void update_pos(vec2 pos) {
    _transform.set_pos(pos);
  }

  template<typename Fun>
  void bind(const window_viewport& window, Fun&& f) {
    _viewport.bind((size_t)window.size.x, (size_t)window.size.y, std::forward<Fun>(f));
  }

  void draw(const window_viewport& window) {
    assert(_viewport.valid());
    const auto& shader = _shader.get();
    const auto sampler = 0;

    shader.use();
    shader.set_uniform("proj", window.proj);
    shader.set_uniform("view", mat4{1.f});
    shader.set_uniform("model", _transform.mat());

    shader.set_uniform("fb_sampler", (int)sampler);
    _viewport.tex().bind_sampler((size_t)sampler);

    renderer::draw_quad();
  }

  void destroy() {
    _viewport.unload();
  }

public:
  const mat4& proj() const { return _proj; }
  const mat4& view() const { return _view; }

private:
  mat4 _proj;
  mat4 _view;
  renderer::framebuffer _viewport;
  ntf::transform2d _transform;
  vec2 _cam_center;
  res::shader _shader;
};

class ui_render {
public:
  ui_render() = default;
  ui_render(const window_viewport& win, res::shader back_shader) : _back_shader(back_shader) {
    const auto& sh = _back_shader.get();

    sh.uniform_location(_proj_u, "proj");
    sh.uniform_location(_model_u, "model");

    sh.uniform_location(_time_u, "time");
    sh.uniform_location(_sampler_u, "tex");

    _ui_root.set_pos((vec2)win.size*.5f).set_scale(win.size);
  }

public:
  void tick(double dt) { // TODO: call this thing in the update loop, somehow
    _back_time += dt;
  }

  void draw_background(const renderer::texture2d& tex, const window_viewport& win) {
    const auto& shader = _back_shader.get();
    const auto sampler = 0;

    shader.use();
    shader.set_uniform(_proj_u, win.proj);
    shader.set_uniform(_model_u, _ui_root.mat());
    shader.set_uniform(_time_u, _back_time);

    shader.set_uniform(_sampler_u, (int)sampler);
    tex.bind_sampler((size_t)sampler);

    renderer::draw_quad();
  }

private:
  res::shader _back_shader;
  float _back_time{0};
  ntf::transform2d _ui_root;
  renderer::shader_uniform _proj_u, _model_u;
  renderer::shader_uniform _time_u, _sampler_u;
};

class sprite_render {
public:
  sprite_render() = default;
  sprite_render(res::shader base_shader) : _base_shader(base_shader) {
    const auto& sh = _base_shader.get();

    sh.uniform_location(_proj_u, "proj");
    sh.uniform_location(_view_u, "view");
    sh.uniform_location(_model_u, "model");
    sh.uniform_location(_offset_u, "offset");
    sh.uniform_location(_color_u, "sprite_color");
    sh.uniform_location(_sampler_u, "sprite_sampler");
  }

public:
  template<typename Obj>
  void draw_thing(Obj&& renderable, const mat4& proj, const mat4& view) const {
    auto& transform = renderable.transform();
    const auto sprite = renderable.sprite();
    const auto& uniforms = renderable.uniforms();

    const auto& shader = _base_shader.get();
    const auto sprite_sampler = 0;

    const auto& atlas = sprite.handle.get();
    shader.use();
    if (uniforms) {
      uniforms.bind(shader);
      // TODO: Find another way to bind samplers?
      renderer::draw_quad();
    } else {
      shader.set_uniform(_proj_u, proj);
      shader.set_uniform(_view_u, view);
      shader.set_uniform(_model_u, transform.mat());
      shader.set_uniform(_color_u, color4{1.f});
      if (sprite.sequence) {
        uint frame = global::state().elapsed_ticks+renderable.birth();
        const auto& seq = atlas.sequence_at(sprite.sequence.value());
        shader.set_uniform(_offset_u, atlas.at(seq[frame%seq.size()]).offset);
      } else {
        shader.set_uniform(_offset_u, atlas.at(sprite.index).offset);
      }
    }

    shader.set_uniform(_sampler_u, (int)sprite_sampler);
    atlas.texture().bind_sampler((size_t)sprite_sampler);

    renderer::draw_quad();
  }

  void draw_sprite(res::sprite sprite, const mat4& model, const mat4& proj, const mat4& view) const {
    const auto& atlas = sprite.handle.get();
    const auto& shader = _base_shader.get();
    const auto sprite_sampler = 0;

    shader.use();
    shader.set_uniform(_proj_u, proj);
    shader.set_uniform(_view_u, view);
    shader.set_uniform(_model_u, model);
    shader.set_uniform(_offset_u, atlas.at(sprite.index).offset);
    shader.set_uniform(_color_u, color4{1.f});

    shader.set_uniform(_sampler_u, (int)sprite_sampler);
    atlas.texture().bind_sampler((size_t)sprite_sampler);

    renderer::draw_quad();
  }

private:
  res::shader _base_shader;
  renderer::shader_uniform _proj_u, _view_u, _model_u;
  renderer::shader_uniform _offset_u, _color_u, _sampler_u; // TODO: get rid of the color unif
};

} // namespace

static window_viewport _window;
static stage_render _stage;
static ui_render _ui;
static sprite_render _renderer;

void render::destroy() {
  _stage.destroy();
}

void render::init(ntf::glfw::window<renderer>& window) {
  // Load OpenGL and things
  renderer::set_blending(true);

  window.set_viewport_event([](size_t w, size_t h) {
    renderer::set_viewport(w, h);
    _window.update(ivec2{w, h});
    _stage.update_pos(vec2(w,h)*.5f);
  });
}

void render::post_init(ntf::glfw::window<renderer>& win) {
  // Prepare shaders and things
  auto fb_shader = res::get_shader("framebuffer");
  auto sprite_shader = res::get_shader("sprite");
  auto front_shader = res::get_shader("frontend");

  auto vp = win.size();
  _window = window_viewport{vp};
  _stage = stage_render{VIEWPORT, VIEWPORT/2, (vec2)vp*0.5f, fb_shader.value()};
  _renderer = sprite_render{sprite_shader.value()};
  _ui = ui_render{_window, front_shader.value()};
}

static void render_gameplay(double dt) {
  _stage.bind(_window, []() {
    renderer::clear_viewport(color3{0.3f});
    auto& stage = global::state().stage;
    auto& player = stage->player;
    auto& boss = stage->boss;

    if (boss.ready()) {
      _renderer.draw_thing(boss, _stage.proj(), _stage.view());
    }

    _renderer.draw_thing(player, _stage.proj(), _stage.view());

    for (auto& bullet : stage->projectiles) {
      _renderer.draw_thing(bullet, _stage.proj(), _stage.view());
    }
  });

  const auto& back = res::texture{0}.get();
  _ui.tick(dt);
  _ui.draw_background(back, _window);

  _stage.draw(_window);
}

static void render_frontend([[maybe_unused]] double dt) {
  // TODO: Move this thing to a widget class
  const auto& back = res::texture{0}.get();
  _ui.tick(dt);
  _ui.draw_background(back, _window);

  const auto& font = res::get_font("arial")->get();
  const auto& font_shader = res::get_shader("font")->get();

  auto& menu = frontend::instance().entry();
  _renderer.draw_sprite(menu.background, menu.back_transform.mat(), _stage.proj(), _stage.view());

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
    font_shader.set_uniform("proj", _window.proj);
    font_shader.set_uniform("model", font_transform.mat());
    font_shader.set_uniform("text_color", col);
    font_shader.set_uniform("tex", (int)shader_sampler);
    font.draw_text(vec2{0.f, 0.f}, 1.f, menu.entries[i].text);
  }
}


void render::draw(window&, [[maybe_unused]] double dt, [[maybe_unused]] double alpha) {
  renderer::clear_viewport(color3{.2f});

  switch (global::state().current_state) {
    case global::states::frontend: {
      render_frontend(dt);
      break;
    }
    case global::states::gameplay: {
      render_gameplay(dt);
      break;
    }
    default: break;
  }
}


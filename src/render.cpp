#include "global.hpp"
#include "render.hpp"
#include "frontend.hpp"

#include <shogle/engine.hpp>

#include <shogle/render/framebuffer.hpp>

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
  stage_render(ivec2 vp_size, ivec2 center, vec2 pos, std::string_view shader_name) :
    _shader(shader_name) {
    update_viewport(vp_size, center);
    update_pos(pos);
  }
  
public:
  void update_viewport(ivec2 vp_size, ivec2 center) {
    _cam_center = (vec2)center;
    _viewport = ntf::framebuffer{(size_t)vp_size.x, (size_t)vp_size.y};
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

    ntf::render_use_shader(shader);
    ntf::render_set_uniform(shader, "proj", window.proj);
    ntf::render_set_uniform(shader, "view", mat4{1.f});
    ntf::render_set_uniform(shader, "model", _transform.mat());
    ntf::render_set_uniform(shader, "fb_sampler", (int)sampler);
    ntf::render_bind_sampler(_viewport.tex(), (size_t)sampler);
    ntf::render_draw_quad();
  }

public:
  const mat4& proj() const { return _proj; }
  const mat4& view() const { return _view; }

private:
  mat4 _proj;
  mat4 _view;
  ntf::framebuffer _viewport;
  ntf::transform2d _transform;
  vec2 _cam_center;
  res::shader _shader;
};

class ui_render {
public:
  ui_render() = default;
  ui_render(const window_viewport& win, std::string_view back_shader) : 
    _back_shader(back_shader) {
    const auto& sh = _back_shader.get();

    _proj_u = sh.uniform_location("proj");
    _model_u = sh.uniform_location("model");

    _time_u = sh.uniform_location("time");
    _sampler_u = sh.uniform_location("tex");

    _ui_root.set_pos((vec2)win.size*.5f).set_scale(win.size);
  }

public:
  void tick(double dt) { // TODO: call this thing in the update loop, somehow
    _back_time += dt;
  }

  void draw_background(const ntf::texture2d& tex, const window_viewport& win) {
    const auto& shader = _back_shader.get();
    const auto sampler = 0;

    ntf::render_use_shader(shader);
    ntf::render_set_uniform(shader, "proj", win.proj);
    ntf::render_set_uniform(shader, "model", _ui_root.mat());
    ntf::render_set_uniform(shader, "time", _back_time);

    ntf::render_set_uniform(shader, "tex", (int)sampler);
    ntf::render_bind_sampler(tex, (size_t)sampler);

    ntf::render_draw_quad();
  }

  void draw_widget(ui::widget* widget) {
    widget->draw();
  }

private:
  res::shader _back_shader;
  float _back_time{0};
  ntf::transform2d _ui_root;
  ntf::shader_uniform _proj_u, _model_u;
  ntf::shader_uniform _time_u, _sampler_u;
};

class sprite_render {
public:
  sprite_render() = default;
  sprite_render(std::string_view shader) : _base_shader(shader) {
    const auto& sh = _base_shader.get();

    _proj_u = sh.uniform_location("proj");
    _view_u = sh.uniform_location("view");
    _model_u = sh.uniform_location("model");

    _offset_u = sh.uniform_location("offset");
    _color_u = sh.uniform_location("sprite_color");
    _sampler_u = sh.uniform_location("sprite_sampler");
  }

public:
  template<typename Obj>
  void draw_thing(Obj&& renderable, const mat4& proj, const mat4& view) {
    auto& transform = renderable.transform();
    const auto sprite = renderable.sprite();
    const auto* renderer = renderable.renderer();

    [[unlikely]] if (renderer != nullptr) { // ?????
      renderer->render(sprite, transform, proj, view);
    } else {
      _draw_thing_default(sprite, transform, proj, view);
    }
  }

  void _draw_thing_default(res::sprite sprite, ntf::transform2d& transform, 
                           const mat4& proj, const mat4& view) {
    const auto& shader = _base_shader.get();
    const auto sprite_sampler = 0;

    ntf::render_use_shader(shader);

    ntf::render_set_uniform(_proj_u, proj);
    ntf::render_set_uniform(_view_u, view);
    ntf::render_set_uniform(_model_u, transform.mat());

    ntf::render_set_uniform(_offset_u, sprite.get_meta().offset);
    ntf::render_set_uniform(_color_u, color4{1.f});

    ntf::render_set_uniform(_sampler_u, (int)sprite_sampler);
    ntf::render_bind_sampler(sprite.get_tex(), (size_t)sprite_sampler);

    ntf::render_draw_quad();
  }

private:
  res::shader _base_shader;
  ntf::shader_uniform _proj_u, _view_u, _model_u;
  ntf::shader_uniform _offset_u, _color_u, _sampler_u; // TODO: get rid of the color unif
};

} // namespace

static window_viewport _window;
static stage_render _stage;
static ui_render _ui;
static sprite_render _renderer;

static void update_viewport(size_t w, size_t h) {
  ntf::render_viewport(w, h);
  _window.update(ivec2{w, h});
  _stage.update_pos(vec2(w,h)*.5f);
}

void render::destroy() {
  ntf::engine_destroy();
}

void render::init() {
  // Load OpenGL and things
  ntf::engine_init(WIN_SIZE.x, WIN_SIZE.y, "test");
  ntf::engine_viewport_event(update_viewport);

  ntf::engine_use_vsync(false);
  ntf::render_blending(true);
}

void render::post_init() {
  // Prepare shaders and things
  auto vp = ntf::engine_window_size();
  _window = window_viewport{vp};
  _stage = stage_render{VIEWPORT, VIEWPORT/2, (vec2)vp*0.5f, "framebuffer"};
  _renderer = sprite_render{"sprite"};
  _ui = ui_render{_window, "frontend"};
}

static void render_gameplay(double dt) {
  _stage.bind(_window, []() {
    ntf::render_clear(color3{0.3f});
    auto& stage = global::state().stage;
    auto& player = stage.player;
    auto& boss = stage.boss;

    if (boss.ready()) {
      _renderer.draw_thing(boss, _stage.proj(), _stage.view());
    }

    _renderer.draw_thing(player, _stage.proj(), _stage.view());

    for (auto& bullet : stage.projectiles) {
      _renderer.draw_thing(bullet, _stage.proj(), _stage.view());
    }
  });

  const auto& back = res::sprite_atlas{}.get();
  _ui.tick(dt);
  _ui.draw_background(back, _window);

  _stage.draw(_window);
}

static void render_frontend(double dt) {
  // TODO: Move this thing to a widget class
  // const auto& back = res::sprite_atlas{}.get(); // give me the default
  // _ui.tick(dt);
  // _ui.draw_background(back, _window);

  const auto& font = res::font{"arial"}.get(); // default
  const auto& font_shader = res::shader{"font"}.get();

  auto& menu = frontend::instance().entry();
  _renderer._draw_thing_default(menu.background, menu.back_transform, 
                                _stage.proj(), _stage.view());

  for (size_t i = 0; i < menu.entries.size(); ++i) {
    const auto focused_index = menu.focused;
    color4 col{1.0f};
    if (i == focused_index) {
      col = color4{1.0f, 0.0f, 0.0f, 1.0f};
    }

    ntf::transform2d font_transform;
    const vec2 pos {100.0f,i*50.0f + 200.0f};
    font_transform.set_pos(pos);

    ntf::render_use_shader(font_shader);
    
    ntf::render_set_uniform(font_shader, "proj", _window.proj);
    ntf::render_set_uniform(font_shader, "model", font_transform.mat());
    ntf::render_set_uniform(font_shader, "text_color", col);

    const auto shader_sampler = 0;
    ntf::render_set_uniform(font_shader, "tex", (int)shader_sampler);
    ntf::render_draw_text(font, vec2{0.0f, 0.0f}, 1.0f, menu.entries[i].text);
  }
}


void render::draw([[maybe_unused]] double dt, [[maybe_unused]] double alpha) {
  ntf::render_clear(color3{0.2f});

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


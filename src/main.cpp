#include "renderer.hpp"

using namespace ntf;

int main() {
  log::set_level(loglevel::verbose);
  shogle::engine eng{800, 600, "test"};

  shogle::quad quad{};

  shogle::sprite_shader shader2d{};
  shogle::model_shader shader3d{};

  shogle::spritesheet sheet{"res/spritesheets/2hus.json"};

  auto& rin_sprite = sheet["rin_dance"];
  sprite_renderer rin_renderer{shader2d, rin_sprite, quad};

  auto& cino_sprite = sheet["cirno_fall"];
  sprite_renderer cino_renderer{shader2d, cino_sprite, quad};

  shogle::model cirno_fumo{"res/models/cirno_fumo/cirno_fumo.obj"};
  model_renderer fumo_renderer{shader3d, cirno_fumo};

  auto cam2d = shogle::camera2d{eng.window().size()}
    .set_pos(0.0f, 0.0f)
    .set_rot(0.0f)
    .set_zoom(1.0f);
  cam2d.update();

  auto cam3d = shogle::camera3d{eng.window().size()}
    .set_pos(0.0f, 0.0f, 0.0f)
    .set_dir(0.0f, 0.0f, -1.0f);
  cam3d.update();

  auto rin_transform = shogle::transform2d{}
    .set_pos(0.5f*(vec2)eng.window().size())
    .set_rot(0.0f)
    .set_scale(200.0f*rin_sprite.corrected_scale());
  rin_transform.update();

  auto cino_transform = shogle::transform2d{}
    .set_pos(0.5f*(vec2)eng.window().size())
    .set_rot(0.0f)
    .set_scale(200.0f*cino_sprite.corrected_scale());
  cino_transform.update();

  auto fumo_transform = shogle::transform3d{}
    .set_pos(0.0f, -0.25f, -1.0f)
    .set_scale(0.015f)
    .set_rot(quat{1.0f, vec3{0.0f}});
  fumo_transform.update();

  size_t rin_index {0};
  size_t cino_index {0};

  eng.set_draw_event([&]() {
    shogle::render_clear(color3{0.2f}, shogle::clear::depth);

    shogle::render_depth_test(true);
    fumo_renderer(cam3d, fumo_transform);

    shogle::render_depth_test(false);
    rin_renderer(cam2d, rin_transform, rin_index);
    cino_renderer(cam2d, cino_transform, cino_index);
  });

  float t = 0.0f;
  float t2 = 0.0f;
  float t3 = 0.0f;
  eng.set_update_event([&](float dt) {
    auto half_screen = 0.5f*(cmplx)eng.window().size();
    t += dt;
    rin_transform.set_rot(rin_transform.rot() + PI*dt).update();
    rin_transform.set_pos(half_screen + 200.0f*math::expic(PI*t));

    cino_transform.set_pos(half_screen).update();

    fumo_transform.set_rot(fumo_transform.rot()*math::axisquat(PI*dt, vec3{0.0f, 1.0f, 0.0f})).update();

    t2 += dt;
    if (t2 > 1/10.0f) {
      rin_index++;
      t2 = 0.0f;
    }

    t3 += dt;
    if (t3 > 1/10.0f) {
      cino_index++;
      t3 = 0.0f;
    }
  });

  eng.set_viewport_event([&](size_t w, size_t h) {
    shogle::render_viewport(w, h);
    cam2d.set_viewport(w, h).update();
    cam3d.set_viewport(w, h).update();
  });

  eng.set_key_event([&](shogle::keycode code, auto, shogle::keystate state, auto) {
    if (code == shogle::key_escape && state == shogle::press) {
      eng.window().close();
    }
  });

  eng.start();
}


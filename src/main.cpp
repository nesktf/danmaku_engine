#include <shogle/shogle.hpp>

#include <shogle/res/spritesheet.hpp>
#include <shogle/res/model.hpp>

#include <shogle/res/shader/sprite.hpp>
#include <shogle/res/shader/model.hpp>

#include <shogle/res/mesh/quad.hpp>

using namespace ntf;

int main() {
  log::set_level(loglevel::LOG_VERBOSE);
  shogle::engine eng{800, 600, "test"};

  shogle::sprite_shader shader2d{};
  shogle::quad_mesh quad{};

  shogle::model_shader shader3d{};

  shogle::spritesheet sheet{"res/spritesheets/2hus.json"};
  auto& rin = sheet["rin_dance"];
  auto& cino = sheet["cirno_fall"];

  shogle::model model{"res/models/cirno_fumo/cirno_fumo.obj"};
  auto& mesh = model[0].mesh;
  auto& diffuse = model[0].find_material(shogle::material::diffuse);

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
    .set_scale(200.0f*rin.corrected_scale);
  rin_transform.update();

  auto cino_transform = shogle::transform2d{}
    .set_pos(0.5f*(vec2)eng.window().size())
    .set_rot(0.0f)
    .set_scale(200.0f*cino.corrected_scale);
  cino_transform.update();

  auto fumo_transform = shogle::transform3d{}
    .set_pos(0.0f, -0.25f, -1.0f)
    .set_scale(0.015f)
    .set_rot(quat{1.0f, vec3{0.0f}});
  fumo_transform.update();

  size_t rin_index {0};
  size_t cino_index {0};

  eng.set_draw_event([&]() {
    shogle::gl::clear_viewport(color3{0.2f}, shogle::gl::clear::depth);

    shogle::gl::set_depth_test(true);
    shader3d.set_proj(cam3d.proj())
      .set_view(mat4{1.0f})
      .set_model(fumo_transform.transf())
      .bind_diffuse(diffuse.tex())
      .draw(mesh);

    shogle::gl::set_depth_test(false);
    shader2d.set_proj(cam2d.proj())
      .set_view(mat4{1.0f})
      .set_transform(rin_transform.transf())
      .set_color(color4{1.0f})
      .set_linear_offset(rin.linear_offset)
      .set_const_offset(rin.get_const_offset(rin_index))
      .bind_texture(sheet.gl_tex())
      .draw(quad);

    shader2d.set_proj(cam2d.proj())
      .set_view(mat4{1.0f})
      .set_transform(cino_transform.transf())
      .set_color(color4{1.0f})
      .set_linear_offset(cino.linear_offset)
      .set_const_offset(cino.get_const_offset(cino_index))
      .bind_texture(sheet.gl_tex())
      .draw(quad);
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
    shogle::gl::set_viewport_size(w, h);
    cam2d.set_viewport(w, h).update();
    cam3d.set_viewport(w, h).update();
  });

  eng.set_key_event([&](shogle::glfw::keycode code, auto, shogle::glfw::keystate state, auto) {
    if (code == shogle::glfw::key_escape && state == shogle::glfw::press) {
      eng.window().close();
    }
  });

  eng.start();
}


#include "renderer.hpp"


using namespace ntf;

int main() {
  log::set_level(loglevel::verbose);
  shogle::engine eng{800, 600, "test"};
  eng.win().use_vsync(false);
  shogle::render_blending(true);

  sprite_renderer render_sprite{};

  std::vector<world_object> danmaku;
  std::vector<world_object> new_danmaku;

  shogle::camera2d cam2d{eng.win().size()};
  cam2d.set_pos(0.0f, 0.0f)
    .set_rot(0.0f)
    .set_zoom(1.0f);
  cam2d.update();

  shogle::tex_filter filter {shogle::tex_filter::nearest};
  shogle::tex_wrap wrap {shogle::tex_wrap::repeat};

  shogle::spritesheet hus_sheet = shogle::load_spritesheet("res/spritesheets/2hus.json", filter, wrap);
  shogle::sprite& rin_sprite = hus_sheet["rin_dance"];

  shogle::spritesheet effects_sheet = shogle::load_spritesheet("res/spritesheets/effects.json", filter, wrap);
  shogle::sprite& star_sprite = effects_sheet["stars_big"];
  shogle::sprite& star_small_sprite = effects_sheet["stars_small"];

  shogle::spritesheet chara_sheet = shogle::load_spritesheet("res/spritesheets/chara.json", filter, wrap);
  shogle::sprite& player_idle = chara_sheet["marisa_idle"];
  shogle::sprite& player_move = chara_sheet["marisa_move"];

  world_object rin {&rin_sprite};
  rin.transform.set_pos(0.0f, 0.0f)
    .set_rot(0.0f)
    .set_scale(200.0f*rin_sprite.corrected_scale())
    .update();

  world_object player {&player_idle};
  player.transform.set_pos(0.0f, 0.0f)
    .set_rot(0.0f)
    .set_scale(60.0f*player_idle.corrected_scale())
    .update();

  shogle::model fumo = shogle::load_model("res/models/cirno_fumo/cirno_fumo.obj", filter, wrap);
  shogle::model_shader fumo_shader{};

  shogle::transform3d fumo_transform{};
  fumo_transform.set_pos(0.0f, -0.25f, -1.0f)
    .set_scale(0.015f)
    .update();

  shogle::camera3d fumo_cam{eng.win().size()};
  fumo_cam.set_pos(0.0f, 0.0f, 0.0f)
    .set_dir(0.0f, 0.0f, -1.0f)
    .update();

  auto on_render = [&](shogle::window& win, double dt, double alpha) {
    shogle::render_clear(color3{0.2f}, shogle::clear::depth);

    shogle::render_depth_test(true);
    for (const auto& [name, mesh] : fumo) {
      fumo_shader.set_proj(fumo_cam.proj())
        .set_view(mat4{1.0f})
        .set_model(fumo_transform.transf())
        .bind_diffuse(mesh[shogle::material_type::diffuse])
        .draw(mesh.get_mesh());
    }

    shogle::render_depth_test(false);
    render_sprite(cam2d, rin);
    render_sprite(cam2d, player);
    for (auto& bullet : danmaku) {
      render_sprite(cam2d, bullet);
    } 

    ImGui::Begin("dou");
    ImGui::Text("fps: %f, alpha: %f, danmaku: %li", 1/dt, alpha, danmaku.size());
    ImGui::End();
  };

  float t = 0.0f;
  float t2 = 0.0f;
  float phase = 0.0f;
  auto on_fixed_update = [&](shogle::window& win, double dt) {
    t += dt;
    t2 += dt;
    phase += dt*PI*0.25f;

    fumo_transform.set_rot(fumo_transform.rot()*math::axisquat(PI*dt, vec3{0.0f,1.0f,0.0f}))
      .update();

    for (auto& obj : new_danmaku) {
      danmaku.push_back(std::move(obj));
    }
    new_danmaku.clear();

    std::erase_if(danmaku, [](const auto& bullet) { return bullet.del; });

    if (t > 1/10.0f) {
      rin.index++;
      player.index++;
      t = 0.0f;
      for (size_t i = 0; i < 8; ++i) {
        world_object bullet {&star_small_sprite};
        bullet.vel = 200.0f*vec2{sin(phase - i*2*PI/8), cos(phase - i*2*PI/8)};
        bullet.index = rand() % 10;
        bullet.ang_speed = PI;
        bullet.transform.set_pos(rin.transform.pos())
          .set_scale(20.0f)
          .update();
        new_danmaku.push_back(std::move(bullet));
      }
    }

    if (t2 > 1/4.0f) {
      for (size_t i = 0; i < 8; ++i) {
        world_object bullet {&star_sprite};
        bullet.vel = 150.0f*vec2{cos(phase + i*2*PI/8), sin(phase + i*2*PI/8)};
        bullet.index = rand() % 10;
        bullet.ang_speed = -PI*0.5f;
        bullet.transform.set_pos(rin.transform.pos())
          .set_scale(50.0f)
          .update();
        new_danmaku.push_back(std::move(bullet));
      }
      t2 = 0.0f;
    }

    for (auto& bullet : danmaku) {
      auto pos = bullet.transform.pos();
      pos += bullet.vel*(float)dt;
      if (pos.x > 800.0f || pos.x < -800.0f || pos.y > 800.0f || pos.y < -800.0f) {
        bullet.del = true;
      }
      bullet.transform.set_pos(pos)
        .set_rot(bullet.transform.rot() + bullet.ang_speed*dt)
        .update();
    }

    float speed = 380.0f*dt;
    if (win.get_key(shogle::key_l)) {
      speed *= 0.66f;
    }
    vec2 vel {0.0f};
    if (win.get_key(shogle::key_a)) {
      vel.x = -1.0f;
    } else if (win.get_key(shogle::key_d)) {
      vel.x = 1.0f;
    }
    if (win.get_key(shogle::key_w)) {
      vel.y = -1.0f;
    } else if (win.get_key(shogle::key_s)) {
      vel.y = 1.0f;
    }
    if (glm::length(vel) > 0) {
      vel = speed*glm::normalize(vel);
    }

    player.transform.set_pos(player.transform.pos() + vel)
      .update();

    rin.transform.update();
  };

  eng.set_viewport_event([&](size_t w, size_t h) {
    shogle::render_viewport(w, h);
    cam2d.set_viewport(w, h).update();
    fumo_cam.set_viewport(w, h).update();
  });

  eng.set_key_event([&](shogle::keycode code, auto, shogle::keystate state, auto) {
    if (code == shogle::key_escape && state == shogle::press) {
      eng.win().close();
    }
    if (code == shogle::key_k && state == shogle::press) {
      for (auto& bullet : danmaku) {
        bullet.del = true;
      }
    }
  });

  eng.start(60, on_render, on_fixed_update);
}


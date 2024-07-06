#include <shogle/shogle.hpp>

#include <shogle/res/spritesheet.hpp>
#include <shogle/res/model.hpp>
#include <shogle/res/font.hpp>

#include <shogle/res/shader/sprite.hpp>
#include <shogle/res/shader/model.hpp>
#include <shogle/res/shader/font.hpp>

#include "danmaku.hpp"

using namespace ntf;

int main() {
  log::set_level(loglevel::verbose);

  shogle::engine_init(1024, 768, "test");
  ntf::cleanup _ {[]{shogle::engine_destroy();}};

  shogle::engine_use_vsync(false);
  shogle::render_blending(true);

  shogle::sprite_shader sprite_shader{};
  shogle::camera2d cam2d{shogle::engine_window_size()};
  cam2d.set_pos(0.0f, 0.0f)
    .set_rot(0.0f)
    .set_zoom(1.0f)
    .update();

  mat4 proj = glm::ortho(0.0f, 1024.0f, 0.0f, 768.0f);
  auto filter = shogle::tex_filter::nearest;
  auto wrap = shogle::tex_wrap::repeat;

  auto effects_sheet = shogle::load_spritesheet("res/spritesheets/effects.json", filter, wrap);
  auto& star_sprite = effects_sheet["stars_big"];

  auto arial = shogle::load_font("res/fonts/arial.ttf");
  shogle::font_shader font_shader{};

  danmaku_spawner danmaku {&star_sprite, 0};
  danmaku.radius = 100.0f;
  danmaku.rate = 10.0f;
  danmaku.shoot_angle_spread = PI*0.25f;
  danmaku.shoot_angle_speed = PI*0.25f;
  danmaku.count = 8;

  // float t = 0.0f;
  vec2 player_pos {0.0f, 0.0f};
  bool pause = false;

  int count = 1;
  int index = 0;
  float radius = 0.0f;
  float rate = 1.0f;
  float angle_spread = 0.0f;
  float angle_speed = 0.0f;
  float bullet_speed = 50.0f;
  float angle = 0.0f;

  auto on_render = [&](double dt, double fixed_dt, double alpha) {
    shogle::render_clear(color3{0.2f});

    danmaku.render(sprite_shader, cam2d);

    font_shader.enable()
      .set_proj(proj)
      .set_color(color4{1.0f,0.0f,0.0f,1.0f})
      .bind_sampler();
    shogle::render_draw_text(arial, "danmaku", {100.0f, 100.0f}, 1.0f);
    std::string coso = std::to_string(danmaku.size());
    shogle::render_draw_text(arial, coso, {305.0f, 100.0f}, 1.0f);

    ImGui::Begin("info");
    ImGui::Text("fps: %f, alpha: %f, danmaku: %li", 1/dt, alpha, danmaku.size());
    ImGui::SliderInt("count", &count, 1, 16);
    ImGui::SliderFloat("angle", &angle, 0.0f, 2*PI);
    ImGui::SliderFloat("radius", &radius, 0.0f, 500.0f);
    ImGui::SliderFloat("rate", &rate, 1.0f, 60.0f);
    ImGui::SliderFloat("spread", &angle_spread, 0.0f, 2*PI);
    ImGui::SliderFloat("ang speed", &angle_speed, -2*PI, 2*PI);
    ImGui::SliderFloat("bullet speed", &bullet_speed, 1.0f, 1000.0f);
    ImGui::SliderInt("index", &index, 0, star_sprite.count());
    if (ImGui::Button("Clear")) {
      danmaku.clear();
    }
    if (ImGui::Button("AutoSpread")) {
      angle_spread = 2*PI/count;
    }
    if (ImGui::Button("NoSpeed")) {
      angle_speed = 0.0f;
    }
    ImGui::End();
  };

  auto on_fixed_update = [&](double dt) {
    danmaku.count = (size_t)count;
    danmaku.radius = radius;
    danmaku.rate = rate;
    danmaku.shoot_angle_spread = angle_spread;
    danmaku.shoot_angle_speed = angle_speed;
    if (angle_speed == 0.0f) {
      danmaku.shoot_angle = angle;
    }

    if (pause) return;

    enum state : int {
      STATE_INIT = 0,
      STATE_MOVING,
      STATE_RETURNING,
    };

    // t+=dt;
    // danmaku.pos = cmplx{player_pos.x, player_pos.y} + 200.0f*math::expic(2*PI*t);

    danmaku.update(dt, [&](auto& bullet) {
      float limit {1000.0f};
      auto pos {bullet.transform.pos()};
      if (pos.x > limit || pos.x < -limit || pos.y > limit || pos.y < -limit) {
        bullet.expired = true;
        return;
      }

      switch (bullet.state) {
        case STATE_INIT: {
          bullet.index = index;
          bullet.dir *= bullet_speed;
          bullet.velfun = [](float t, cmplx dir) -> cmplx {
            // float s = 200.0f;
            return dir*cmplx{1.0f};
            // return math::expic(PI*0.25+t)*dir*s*cmplx{1/(t+1), t+3.5f};
            // return init + math::expic(t)*dir*cmplx{speed*t, radius*glm::log(.082f+t/8)};
          };
          bullet.next_state = STATE_MOVING;
          break;
        }
        // case STATE_MOVING: {
        //   if (bullet.state_t > 1.0f) {
        //     bullet.index++;
        //     vec2 d = glm::normalize(player_pos-bullet.transform.pos());
        //     bullet.dir = cmplx{d.x, d.y};
        //     bullet.velfun = [](float t, cmplx dir) -> cmplx {
        //       return dir*cmplx{5.0f*t};
        //     };
        //     // bullet.dir *= math::expic(PI*0.5f);
        //     bullet.next_state = STATE_RETURNING;
        //   }
        // }
        default: break;
      }
    });
  };

  shogle::engine_viewport_event([&](size_t w, size_t h) {
    shogle::render_viewport(w, h);
    cam2d.set_viewport(w, h).update();
  });

  shogle::engine_key_event([&](shogle::keycode code, auto, shogle::keystate state, auto) {
    if (code == shogle::key_escape && state == shogle::press) {
      shogle::engine_close_window();
    }
    if (code == shogle::key_k && state == shogle::press) {
      danmaku.clear();
    }
    if (code == shogle::key_j && state == shogle::press) {
      cam2d.set_zoom(cam2d.zoom()+0.1f);
    }
    if (code == shogle::key_l && state == shogle::press) {
      cam2d.set_zoom(cam2d.zoom()-0.1f);
    }
    if (code == shogle::key_space && state == shogle::press) {
      pause = !pause;
    }
    cam2d.update();
  });

  shogle::engine_main_loop(60, on_render, on_fixed_update);
}


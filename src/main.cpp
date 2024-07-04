#include <shogle/shogle.hpp>

#include <shogle/res/spritesheet.hpp>
#include <shogle/res/model.hpp>

#include <shogle/res/shader/sprite.hpp>
#include <shogle/res/shader/model.hpp>

#include <shogle/res/meshes.hpp>

#include "danmaku.hpp"

using namespace ntf;

static cmplx defvel(float t, cmplx dir) { return cmplx{}; }

struct bullet {
  using velf = cmplx(*)(float,cmplx);

  bullet(size_t index_, cmplx pos_, velf velfun_, cmplx dir_) :
    index(index_), velfun(velfun_), dir(dir_) { transform.set_pos(pos_); }

  size_t index;
  velf velfun;
  cmplx dir;

  shogle::transform2d transform{};
  float t{0.0f};
  float state_t{0.0f};
  int state{0};
  int next_state{0};
  bool expired{false};
};

class danmaku_spawner {
public:
  danmaku_spawner(shogle::sprite* sprite, size_t index, bullet::velf def_velfun = defvel) :
    _sprite(sprite), _index(index), _def_velfun(def_velfun) {}

public:
  template<typename StatePred>
  void update(float dt, StatePred&& state_pred) {
    _t += dt;
    shoot_angle += shoot_angle_speed*dt;

    for (auto& bullet : _new_danmaku) {
      _danmaku.emplace_back(std::move(bullet));
    }
    _new_danmaku.clear();

    for (auto& bullet : _danmaku) {
      bullet.t += dt;
      bullet.state_t += dt;

      state_pred(bullet);

      if (bullet.state != bullet.next_state) {
        bullet.state = bullet.next_state;
        bullet.state_t = 0.0f;
      }

      auto& t = bullet.transform;
      auto& v = bullet.velfun;
      t.set_pos(t.cpos() + v(bullet.t, bullet.dir)*dt)
        .set_rot(t.rot() + PI*dt)
        .set_scale(40.0f);
    }

    std::erase_if(_danmaku, [](const auto& bullet) { return bullet.expired; });

    if (_t > 1/rate) {
      _t = 0.0f;

      for (size_t i = 0; i < count; ++i) {
        cmplx dir = math::expic(shoot_angle + (i*shoot_angle_spread));
        cmplx p = pos + dir*cmplx{radius, 0.0f};
        _new_danmaku.emplace_back(_index, p, _def_velfun, dir);
      }
    }
  }

  void render(shogle::sprite_shader& shader, const shogle::camera2d& cam, 
              const shogle::mesh& quad) {
    for (auto& bullet : _danmaku) {
      shader.set_proj(cam.proj())
        .set_view(cam.view())
        .set_transform(bullet.transform.mat())
        .set_color(color4{1.0f})
        .set_tex_offset(_sprite->tex_offset(bullet.index))
        .bind_texture(_sprite->tex())
        .draw(quad);
    }
  }

public:
  size_t size() const { return _danmaku.size(); }

  void clear() { _danmaku.clear(); }

public:
  cmplx pos{0.0f};

  float rate {1.0f};
  float shoot_angle {0.0f};
  float shoot_angle_spread {0.0f};
  float shoot_angle_speed {0.0f};
  float radius{0.0f};

  size_t count {1};

private:
  shogle::sprite* _sprite;
  size_t _index;
  bullet::velf _def_velfun;

  std::vector<bullet> _danmaku;
  std::vector<bullet> _new_danmaku;
  float _t{0.0f};
};

int main() {
  log::set_level(loglevel::verbose);

  shogle::engine eng{1024, 768, "test"};
  eng.win().use_vsync(false);
  shogle::render_blending(true);

  auto quad = shogle::load_quad(shogle::quad_type::normal2d);
  shogle::sprite_shader sprite_shader{};

  shogle::camera2d cam2d{eng.win().size()};
  cam2d.set_pos(0.0f, 0.0f)
    .set_rot(0.0f)
    .set_zoom(1.0f)
    .update();

  auto filter = shogle::tex_filter::nearest;
  auto wrap = shogle::tex_wrap::repeat;

  auto effects_sheet = shogle::load_spritesheet("res/spritesheets/effects.json", filter, wrap);
  auto& star_sprite = effects_sheet["stars_big"];

  danmaku_spawner danmaku {&star_sprite, 0};
  danmaku.radius = 100.0f;
  danmaku.rate = 10.0f;
  danmaku.shoot_angle_spread = PI*0.25f;
  danmaku.shoot_angle_speed = PI*0.25f;
  danmaku.count = 8;

  auto on_render = [&](shogle::window& win, double dt, double fixed_dt, double alpha) {
    shogle::render_clear(color3{0.2f});

    danmaku.render(sprite_shader, cam2d, quad);

    ImGui::Begin("info");
    ImGui::Text("fps: %f, alpha: %f, danmaku: %li", 1/dt, alpha, danmaku.size());
    ImGui::End();
  };

  vec2 player_pos {0.0f, 0.0f};

  float t = 0.0f;
  auto on_fixed_update = [&](shogle::window& win, double dt) {
    enum state : int {
      STATE_INIT = 0,
      STATE_MOVING,
      STATE_RETURNING,
    };

    t+=dt;
    danmaku.pos = cmplx{0.0f, 0.0f} + 200.0f*math::expic(2*PI*t);

    danmaku.update(dt, [&](bullet& bullet) {
      float limit {1000.0f};
      auto pos {bullet.transform.pos()};
      if (pos.x > limit || pos.x < -limit || pos.y > limit || pos.y < -limit) {
        bullet.expired = true;
        return;
      }

      switch (bullet.state) {
        case STATE_INIT: {
          bullet.velfun = [](float t, cmplx dir) -> cmplx {
            float s = 200.0f;
            return math::expic(PI*0.25+t)*dir*s*cmplx{1/(t+1), t+3.5f};
            // return init + math::expic(t)*dir*cmplx{speed*t, radius*glm::log(.082f+t/8)};
          };
          bullet.next_state = STATE_MOVING;
          break;
        }
        case STATE_MOVING: {
          if (bullet.state_t > 1.0f) {
            bullet.index++;
            vec2 d = glm::normalize(player_pos-bullet.transform.pos());
            bullet.dir = cmplx{d.x, d.y};
            bullet.velfun = [](float t, cmplx dir) -> cmplx {
              return dir*cmplx{5.0f*t};
            };
            // bullet.dir *= math::expic(PI*0.5f);
            bullet.next_state = STATE_RETURNING;
          }
        }
        default: break;
      }
    });
  };

  eng.set_viewport_event([&](size_t w, size_t h) {
    shogle::render_viewport(w, h);
    cam2d.set_viewport(w, h).update();
  });

  eng.set_key_event([&](shogle::keycode code, auto, shogle::keystate state, auto) {
    if (code == shogle::key_escape && state == shogle::press) {
      eng.win().close();
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
    cam2d.update();
  });

  eng.start(60, on_render, on_fixed_update);
}


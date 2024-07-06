#pragma once

#include <shogle/shogle.hpp>
#include <shogle/res/spritesheet.hpp>

#include <shogle/res/shader/sprite.hpp>

#include <cstdlib>

namespace ntf {

class danmaku_spawner {
public:
  using velf = cmplx(*)(float,cmplx);

  struct bullet {
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

public:
  danmaku_spawner(shogle::sprite* sprite, size_t index, velf def_velfun = [](auto,auto){return cmplx{};}) :
    _sprite(sprite), _index(index), _def_velfun(def_velfun) {}

public:
  template<typename StatePred>
  void update(float dt, StatePred&& state_pred) {
    _t += dt;
    shoot_angle += shoot_angle_speed*dt;

    if (_t > 1/rate) {
      _t = 0.0f;

      for (size_t i = 0; i < count; ++i) {
        cmplx dir = math::expic(shoot_angle + (i*shoot_angle_spread));
        cmplx p = pos + dir*cmplx{radius, 0.0f};
        _danmaku.emplace_back(_index, p, _def_velfun, dir);
      }
    }

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
  }

  void render(shogle::sprite_shader& shader, const shogle::camera2d& cam) {
    for (auto& bullet : _danmaku) {
      shader.enable()
        .set_proj(cam.proj())
        .set_view(cam.view())
        .set_transform(bullet.transform.mat())
        .set_color(color4{1.0f})
        .set_tex_offset(_sprite->tex_offset(bullet.index))
        .bind_texture(_sprite->tex());
      shogle::render_draw_quad();
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
  velf _def_velfun;

  std::vector<bullet> _danmaku;
  float _t{0.0f};
};



// inline void player_mover(shogle::transform2d& transform, float dt) {
//   float speed = 380.0f*dt;
//   if (win.get_key(shogle::key_l)) {
//     speed *= 0.66f;
//   }
//   vec2 vel {0.0f};
//   if (win.get_key(shogle::key_a)) {
//     vel.x = -1.0f;
//   } else if (win.get_key(shogle::key_d)) {
//     vel.x = 1.0f;
//   }
//   if (win.get_key(shogle::key_w)) {
//     vel.y = -1.0f;
//   } else if (win.get_key(shogle::key_s)) {
//     vel.y = 1.0f;
//   }
//   if (glm::length(vel) > 0) {
//     vel = speed*glm::normalize(vel);
//   }
//   transform.set_pos(transform.pos() + vel);
// }

} // namespace ntf

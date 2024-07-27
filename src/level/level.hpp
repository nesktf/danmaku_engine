#pragma once

#include "render.hpp"

namespace ntf::game {

struct level {
  virtual void render() = 0;
  virtual void tick(float dt) = 0;
  virtual ~level() = default;
};

struct renderable {
  using container = std::vector<uptr<renderable>>;

  virtual void render() = 0;
  virtual void tick(container& danmaku, float dt) = 0;
  virtual ~renderable() = default;

  bool expired{false};
};

template<typename state_type, typename state_handler>
class enemy : public renderable {
public:
  enemy(const shogle::sprite* sprite, state_handler handler, state_type init_state) :
    _sprite(sprite), _state_handler(std::move(handler)), _state(init_state) {
      _transf.set_scale(50.0f);
    }

public:
  void render() override { render_sprite(*_sprite, _transf, 0); }

  void tick(renderable::container& danmaku, float dt) override {
    _t += dt;
    _state_handler(*this, danmaku, dt);
  }

public:
  shogle::transform2d& transform() { return _transf; }
  float time() const { return _t; }
  state_type state() const { return _state; }
  void set_state(state_type state) { _state = state; }

public:
  const shogle::sprite* _sprite;
  state_handler _state_handler;

  shogle::transform2d _transf{};
  float _t{};
  state_type _state{};
};


class boss {
public:
  boss(const shogle::sprite* sprite, cmplx pos) : _sprite(sprite) {
    _transf.set_pos(pos)
      .set_scale(68.0f*sprite->corrected_scale());
  }

public:
  void render() {
    render_sprite(*_sprite, _transf, 0);
  }

  template<typename Pred>
  void tick(float dt, Pred&& pred) {
    pred(*this, dt);
  }

public:
  const shogle::sprite* _sprite;
  shogle::transform2d _transf{};
};

template<typename state_component>
class entity : public renderable {
public:
  entity(state_component state_) :
    state(std::move(state)) {
    state.init(*this);
  }
public:
  using transform_fun = void(*)(const state_component&,shogle::transform2d&,float,float);

  void tick(renderable::container&, float dt) override {
    t += dt;
    state.tick(*this, dt);
    transf_fun(state, transform, t, dt);
  }

public:
  const shogle::sprite* sprite;
  state_component state;
  transform_fun transf_fun;

  shogle::transform2d transform{};
  float t {0.0f};
};


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

// class bezier_path {
// public:
//   using bez_fun = vec2(*)(float, const vec4&, const vec4&);
//
//   static vec2 bez_lin(float t, const vec4& p_ends, const vec4& p_inter) {
//     const auto p0 = vec2{p_ends.x, p_ends.y};
//     const auto p1 = vec2{p_ends.z, p_ends.w};
//
//     return p0 + t*(p1 - p0);
//   }
//
//   static vec2 bez_qua(float t, const vec4& p_ends, const vec4& p_inter) {
//     const auto p0 = vec2{p_ends.x, p_ends.y};
//     const auto p1 = vec2{p_inter.x, p_inter.y};
//     const auto p2 = vec2{p_inter.z, p_inter.w};
//
//     return ((1-t)*(1-t)*p0) + (2*(1-t)*t*p1) + (t*t*p2);
//   }
//
//   static vec2 bez_cub(float t, const vec4& p_ends, const vec4& p_inter) {
//     const auto p0 = vec2{p_ends.x, p_ends.y};
//     const auto p1 = vec2{p_inter.x, p_inter.y};
//     const auto p2 = vec2{p_inter.z, p_inter.w};
//     const auto p3 = vec2{p_ends.z, p_ends.w};
//
//     return ((1-t)*(1-t)*(1-t)*p0) + (3*(1-t)*(1-t)*t*p1) + (3*(1-t)*t*t*p2) + (t*t*t*p3);
//   }
//
// public:
//   bezier_path(vec2 p0, vec2 p1) :
//     _ends(p0, p1), _fun(bez_lin) {}
//
//   bezier_path(vec2 p0, vec2 p1, vec2 p2) :
//     _ends(p0, p2), _inter(p2, vec2{0.0f}), _fun(bez_qua) {}
//
//   bezier_path(vec2 p0, vec2 p1, vec2 p2, vec2 p3) :
//     _ends(p0, p3), _inter(p1, p2), _fun(bez_cub) {}
//
// public:
//   vec2 operator()(float t) { return _fun(t, _ends, _inter); }
//
// private:
//   const vec4 _ends, _inter{};
//   bez_fun _fun;
// };

} // namespace ntf::game

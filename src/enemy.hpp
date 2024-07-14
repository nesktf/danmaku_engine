#pragma once

#include <shogle/res/spritesheet.hpp>
#include <shogle/scene/transform.hpp>

namespace ntf::game {

class enemy {
public:
  using velf = cmplx(*)(float,cmplx);

public:
  enemy(shogle::sprite* sprite, velf velfun, cmplx pos, cmplx dir);

public:
  void render();

  template<typename Pred>
  void tick(float dt, Pred&& pred) {
    _t += dt;
    _transf.set_pos(_transf.cpos() + _velfun(_t, _dir)*dt);
    _dead = pred(*this);
  }
  
public:
  bool dead() const { return _dead; }
  vec2 pos() const { return _transf.pos(); }

private:
  shogle::sprite* _sprite;
  velf _velfun;
  cmplx _dir;
  float _t{};
  shogle::transform2d _transf{};
  bool _dead{false};
};

}

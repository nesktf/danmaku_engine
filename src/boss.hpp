#pragma once

#include <shogle/res/spritesheet.hpp>
#include <shogle/scene/transform.hpp>

namespace ntf::game {

class boss {
public:
  boss(shogle::sprite* sprite, cmplx pos);

public:
  void render();

  template<typename Pred>
  void tick(float dt, Pred&& pred) {
    pred(*this, dt);
  }

public:
  shogle::sprite* _sprite;
  shogle::transform2d _transf{};
};

}

#pragma once

#include <shogle/res/spritesheet.hpp>
#include <shogle/scene/transform.hpp>

namespace ntf::game {

class player_state {
public:
  static const constexpr auto base_scale = 50.0f;

public:
  player_state();

public:
  void render();
  void tick(float dt);

public:
  void set_sprite(shogle::sprite* sprite, shogle::sprite* hitbox);

public:
  vec2 pos() const { return _transf.pos(); }

private:
  shogle::sprite* _sprite;
  shogle::sprite* _hitbox;
  shogle::transform2d _transf{};
  shogle::transform2d _hitbox_transform{};

  bool _shifting {false};
};

}

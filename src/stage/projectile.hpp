#pragma once

#include "stage/entity.hpp"

#include "stage/movement.hpp"
#include "stage/animator.hpp"

namespace okuu::stage {

class projectile : public entity {
public:
  projectile(frames birth, ntf::transform2d transf, movement move, animator anim);

public:
  void tick();
  
public:
  movement move;
  real ang_speed;

  animator anim;

  bool cflag;
};


class projectile_view {
public:
  using iterator = std::list<stage::projectile>::iterator;

public:
  projectile_view(std::list<stage::projectile>& list, std::size_t size);

public:
  void for_each(sol::function f);

public:
  std::size_t size() const { return _size; }

private:
  iterator _begin;
  std::size_t _size;
};

} // namespace okuu::stage

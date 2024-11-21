#pragma once

#include "stage/entity.hpp"

namespace okuu::stage {

class player : public entity {
public:



public:
  player(frames birth, ntf::transform2d transf, real bspd, real sspd, 
         animator::animation_data anim);

public:
  void tick();

public:
  movement move;
  animator anim;
};

} // namespace okuu::stage

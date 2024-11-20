#pragma once

#include "stage/entity.hpp"

namespace okuu {

class boss_entity {
public:
  void init(okuu::frames birth, ntf::transform2d transform,
            okuu::entity_movement move, okuu::entity_animator anim);

public:
  void tick();

public:
  const ntf::transform2d& transform() const { return _transform; }
  ntf::transform2d& transform() { return  _transform;}

  okuu::sprite sprite() const { return _anim.sprite(); }

  bool flag() const { return _boss_flag; }

private:
  ntf::transform2d _transform;
  okuu::entity_animator _anim;
  okuu::entity_movement _move;
  okuu::frames _birth, _life{0};
  bool _boss_flag{false};
};

} // namespace okuu::stage

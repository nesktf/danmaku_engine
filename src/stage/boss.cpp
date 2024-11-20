#include "./boss.hpp"

namespace okuu {

void boss_entity::init(okuu::frames birth, ntf::transform2d transform,
                       okuu::entity_movement move, okuu::entity_animator anim) {
  NTF_ASSERT(!_boss_flag);
  _birth = birth;
  _move = move;
  _anim = anim;
  _transform = std::move(transform);
  _birth = birth;
  _boss_flag = true;
}

void boss_entity::tick() {
  _move.tick(_transform);
  _anim.tick(++_life);
}

} // namespace okuu

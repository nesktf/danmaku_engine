#include "entity/projectile.hpp"

namespace entity {

projectile::projectile(ntf::shogle::sprite sprite_, movement movement_, uint birth_) :
  sprite(sprite_), move(movement_), birth(birth_) {
  transform.set_scale(20.0f)
    .set_pos(0.0f, 0.0f);
}

void projectile::tick() {
  auto new_pos = transform.cpos();
  move(new_pos);
  transform.set_pos(new_pos)
    .set_rot(transform.rot() + PI/60);
}


} // namespace ntf

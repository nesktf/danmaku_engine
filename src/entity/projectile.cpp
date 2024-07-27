#include "entity/projectile.hpp"

namespace ntf {

projectile::projectile(const shogle::sprite* sprite_, entity_movement movement_) :
  sprite(sprite_), movement(movement_) {
  transform.set_scale(20.0f)
    .set_pos(0.0f, 0.0f);
}

void projectile::tick() {
  auto new_pos = transform.cpos();
  movement(new_pos);
  transform.set_pos(new_pos)
    .set_rot(transform.rot() + PI/60);
}


} // namespace ntf

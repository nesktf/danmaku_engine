#pragma once

#include "core.hpp"

#include "entity/projectile.hpp"
#include "entity/player.hpp"
#include "entity/boss.hpp"

namespace stage {

void init();
void tick();

vector<entity::projectile>& projectiles();
entity::player& player();
entity::boss& boss();

} // namespace stage

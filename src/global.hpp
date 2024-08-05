#pragma once

#include "stage.hpp"

#include "entity/player.hpp"

namespace global {

struct global_state {
  uint frames{0};
  stage::stage_state stage;
};

void init();
void tick();

global_state& state();

} // namespace global

#pragma once

#include "stage.hpp"

#include "entity/player.hpp"

namespace global {

enum class states {
  loading = 0,
  frontend,
  gameplay,
};

struct global_state {
  uint frames{0};
  states current_state{states::loading};
  stage::stage_state stage;
};

void init();
void tick();
void start_stage();
void go_back();

global_state& state();

} // namespace global

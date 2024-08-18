#pragma once

#include "core.hpp"

namespace stage { class state; } // avoid sol propagation

namespace global {

enum class states {
  loading = 0,
  frontend,
  gameplay,
};

struct global_state {
  states current_state{states::loading};
  std::unique_ptr<stage::state> stage;
  frames elapsed_ticks{0};
  frames elapsed_frames{0};
};

void init();
void tick();
void start_stage(std::string path);
void go_back();

global_state& state();

} // namespace global

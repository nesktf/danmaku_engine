#include "global.hpp"

#include "resources.hpp"

namespace global {

static global_state _state;

void init() {
  const char* script_path = "res/script/stage/test.lua";
  _state.stage.load_env(script_path);
}

void tick() {
  _state.frames++;
  _state.stage.tick();
}

global_state& state() { return _state; }

} // namespace global

#include "stage/state.hpp"

#include "global.hpp"
#include "resources.hpp"

#include "render/ui/frontend.hpp"

namespace global {

static global_state _state;

void init() {
  frontend::init();
  _state.current_state = global::states::frontend;
}

void tick() {
  _state.elapsed_ticks++;

  switch(_state.current_state) {
    case global::states::gameplay: {
      assert(_state.stage && "Stage not initialized");
      _state.stage->tick();
      break;
    }
    case global::states::frontend: {
      frontend::tick();
      break;
    }
    default: break;
  }
}

void start_stage(std::string path) {
  _state.stage = std::make_unique<stage::state>(path);
  _state.current_state = global::states::gameplay;
}

void go_back() {
  _state.current_state = global::states::frontend;
}

global_state& state() { return _state; }

} // namespace global

#include "global.hpp"

#include "resources.hpp"
#include "frontend.hpp"

namespace global {

static global_state _state;

void init() {
  frontend::init();
  _state.current_state = global::states::frontend;

}

void tick() {
  if (_state.current_state == global::states::gameplay) {
    _state.frames++;
    _state.stage.tick();
  } else if (_state.current_state == global::states::frontend) {
    frontend::tick();
  }
}

void start_stage(std::string path) {
  _state.stage.load_env(path);
  _state.current_state = global::states::gameplay;
}

void go_back() {
  _state.current_state = global::states::frontend;
}

global_state& state() { return _state; }

} // namespace global

#pragma once

#include "core.hpp"

namespace input {

void init();

inline bool poll_key(keycode key) {
  return ntf::engine_poll_key(key);
}

} // namespace input

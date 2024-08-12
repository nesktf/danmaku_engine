#pragma once

#include "core.hpp"

#include <shogle/engine.hpp>

namespace input {

void init();

inline bool poll_key(ntf::keycode key) {
  return ntf::engine_poll_key(key);
}

} // namespace input

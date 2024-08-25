#pragma once

#include "core.hpp"

#include <shogle/engine.hpp>

namespace input {

using keycode = ntf::glfw::keycode;
using keystate = ntf::glfw::keystate;

void init(ntf::glfw::window<renderer>& window);

bool poll_key(keycode code, keystate state = keystate::press);

} // namespace input

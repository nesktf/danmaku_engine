#pragma once

#include <sol/sol.hpp>

#include "global.hpp"

namespace package {

sol::table stlib_open(sol::state_view lua);

} // namespace package

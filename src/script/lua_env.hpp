#pragma once

#include <sol/sol.hpp>

#include "core.hpp"

namespace stage {

class state;

} // namespace stage


namespace script {

class lua_env {
public:
  lua_env(stage::state* stage, std::string_view script_path);

public:
  frame_delay call_task();

private:
  void _populate_globals(stage::state* stage);

private:
  sol::state _lua;
  sol::protected_function _main_task;
};


} // namespace script

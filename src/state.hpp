#pragma once

#include <shogle/render/framebuffer.hpp>
#include <shogle/res/spritesheet.hpp>
#include <shogle/res/font.hpp>

#include <shogle/scene/camera.hpp>

#include "player.hpp"
#include "level.hpp"

namespace ntf::game {

struct res {
  std::unordered_map<std::string, shogle::spritesheet> spritesheets;
  std::unordered_map<std::string, shogle::font> fonts;
};

extern res& resources;

struct game_state {
  shogle::camera2d camera{vec2{1.0f}};
  player_state player;
  uptr<level> curr_level;
  uptr<shogle::framebuffer> level_fbo;
};

extern game_state& state;

}

#pragma once

#include <shogle/core/types.hpp>

#include <array>

#include "entity/projectile.hpp"
#include "entity/boss.hpp"
#include "entity/player.hpp"

namespace ntf {

const constexpr uint UPS = 60;
const constexpr ivec2 WIN_SIZE {1280, 720};
const constexpr ivec2 VIEWPORT = {880, 660};
// const constexpr ivec2 VIEWPORT {768, 896};
const constexpr auto fb_ratio = 6.f/7.f;

struct global_t {
  // player player;
  std::vector<projectile> projectiles;
  // boss* boss;
};

extern global_t global;

void global_init();


} // namespace ntf

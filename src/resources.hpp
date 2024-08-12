#pragma once

#include "core.hpp"

#include <shogle/render/shader.hpp>

#include <shogle/res/pool.hpp>
#include <shogle/res/spritesheet.hpp>

namespace res {

using spritesheet_id = ntf::resource_id;
using shader_id = ntf::resource_id;

struct sprite_id {
  ntf::spritesheet::sprite index;
  spritesheet_id sheet;
};


void init();

spritesheet_id spritesheet_index(std::string_view name);
const ntf::spritesheet& spritesheet_at(spritesheet_id sheet);
const ntf::spritesheet& spritesheet_at(std::string_view name);

const ntf::spritesheet::sprite_data& sprite_data_at(sprite_id sprite);

shader_id shader_index(std::string_view name);
const ntf::shader_program& shader_at(shader_id shader);
const ntf::shader_program& shader_at(std::string_view name);

} // namespace res

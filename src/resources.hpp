#pragma once

#include "core.hpp"

namespace res {

void init();

const spritesheet& spritesheet(std::string_view name);
const shader_program& shader(std::string_view name);
const font& font(std::string_view name);

} // namespace res

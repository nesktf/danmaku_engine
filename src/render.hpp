#pragma once

#include <shogle/res/spritesheet.hpp>
#include <shogle/res/font.hpp>

#include <shogle/scene/transform.hpp>

namespace ntf::game {

void render_init();

void render_sprite(const shogle::sprite& sprite, shogle::transform2d& transform, 
                   size_t index, const color4& col = color4{1.0f});

void render_text(const shogle::font& font, const vec2& pos, float scale,
                 const color4& col, std::string_view text);

template<typename... Args>
void render_text(const shogle::font& font, const vec2& pos, float scale,
                 const color4& col, fmt::format_string<Args...> fmt, Args&&... args) {
  std::string str = fmt::format(fmt, std::forward<Args>(args)...);
  render_text(font, pos, scale, col, str);
}

}

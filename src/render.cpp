#include "render.hpp"
#include "state.hpp"

#include <shogle/res/shader/sprite.hpp>
#include <shogle/res/shader/font.hpp>

static ntf::uptr<ntf::shogle::sprite_shader> sprite_shader;
static ntf::uptr<ntf::shogle::font_shader> font_shader;

namespace ntf::game {

void render_init() {
  sprite_shader = ntf::make_uptr<shogle::sprite_shader>();
  font_shader = ntf::make_uptr<shogle::font_shader>();
}

void render_sprite(const shogle::sprite& sprite, shogle::transform2d& transform,
                   size_t index, const color4& col) {
  auto& cam = state.camera;
  sprite_shader->enable()
    .set_proj(cam.proj())
    .set_view(cam.view())
    .set_transform(transform.mat())
    .set_color(col)
    .set_tex_offset(sprite.tex_offset(index))
    .bind_texture(sprite.tex());
  shogle::render_draw_quad();
}

void render_text(const shogle::font& font, const vec2& pos, float scale,
                 const color4& col, std::string_view text) {
  auto& cam = state.camera;
  font_shader->enable()
    .set_proj(cam.proj())
    .set_color(col)
    .bind_sampler();
  shogle::render_draw_text(font, pos, scale, text);
}


}

// #include "render.hpp"
// #include "state.hpp"
//
// #include <shogle/core/log.hpp>
//
// #include <shogle/res/shader/sprite.hpp>
// #include <shogle/res/shader/font.hpp>
//
// using namespace ntf;
//
// static uptr<shogle::sprite_shader> sprite_shader;
// static uptr<shogle::font_shader> font_shader;
//
// namespace ntf::game {
//
// void render_init() {
//   sprite_shader = std::make_unique<shogle::sprite_shader>();
//   font_shader = std::make_unique<shogle::font_shader>();
//   log::debug("[game] Render init");
// }
//
// void draw_sprite(shogle::transform2d& transform, const shogle::sprite& sprite, 
//                  const color4& col, size_t index) {
//   sprite_shader->enable()
//     .set_proj(global_state.cam.proj())
//     .set_view(global_state.cam.view())
//     .set_transform(transform.mat())
//     .set_color(col)
//     .set_tex_offset(sprite.tex_offset(index))
//     .bind_texture(sprite.tex());
//   shogle::render_draw_quad();
// }
//
// void draw_text(const shogle::font& font, vec2 pos, float scale, color4 col, std::string_view text) {
//   font_shader->enable()
//     .set_proj(global_state.cam.proj())
//     .set_color(col)
//     .bind_sampler();
//   shogle::render_draw_text(font, pos, scale, text);
// }
//
// } // namespace ntf::game

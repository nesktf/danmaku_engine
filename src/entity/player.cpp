#include "player.hpp"
#include "input.hpp"

namespace entity {

void player::tick() {

}

} // namespace entity

// #include "render.hpp"
// #include "state.hpp"
//
// #include <shogle/shogle.hpp>
//
// namespace ntf::game {
//
// static float sqlen(const vec2& vec) {
//   return (vec.x*vec.x) + (vec.y*vec.y);
// }
//
// player_state::player_state() {}
//
// void player_state::set_sprite(const shogle::sprite* sprite, const shogle::sprite* hitbox) {
//   _hitbox = hitbox;
//   _sprite = sprite;
//
//   _transf.set_scale(base_scale*sprite->corrected_scale());
//   _hitbox_transform.set_scale(base_scale*_hitbox->corrected_scale());
// }
//
// void player_state::render() {
//   render_sprite(*_sprite, _transf, 0);
// }
//
// void player_state::render_hitbox() {
//   if (_shifting) {
//     render_sprite(*_hitbox, _hitbox_transform, 0);
//   }
// }
//
// void player_state::tick(float dt) {
//   _shifting = false;
//
//   float speed = 450.0f;
//   if (shogle::engine_poll_key(shogle::key_l)) {
//     speed *= 0.66f;
//     _shifting = true;
//   }
//
//   vec2 vel{};
//   if (shogle::engine_poll_key(shogle::key_a)) {
//     vel.x = -1.0f;
//   } else if (shogle::engine_poll_key(shogle::key_d)) {
//     vel.x = 1.0f;
//   }
//   if (shogle::engine_poll_key(shogle::key_w)) {
//     vel.y = -1.0f;
//   } else if (shogle::engine_poll_key(shogle::key_s)) {
//     vel.y = 1.0f;
//   }
//
//   if (sqlen(vel) > 0) {
//     vel = glm::normalize(vel);
//   }
//
//   const auto vp = state.level_info.viewport->size();
//   const auto pos = _transf.pos() + vel*speed*dt;
//   _transf.set_pos(glm::clamp(pos, vec2{0.0f}, vp));
//
//   if (_shifting) {
//     _hitbox_transform.set_pos(_transf.pos())
//       .set_rot(_hitbox_transform.rot() + PI*0.25f*dt);
//   }
//   // state.camera.set_pos(_transf.pos()).update();
// }
//
// }

// #include "player.hpp"
//
// namespace ntf::game {
//
// class movement_component : public component {
// public:
//   movement_component(sprite_component* sprite) :
//     _sprite(sprite) {}
//
// public:
//   void on_tick(float dt) override {
//
//     float speed = 380.0f;
//     if (shogle::engine_poll_key(shogle::key_l)) {
//       speed *= 0.66f;
//     }
//
//     vec2 vel {0.0f};
//     if (shogle::engine_poll_key(shogle::key_a)) {
//       vel.x = -1.0f;
//     } else if (shogle::engine_poll_key(shogle::key_d)) {
//       vel.x = 1.0f;
//     }
//     if (shogle::engine_poll_key(shogle::key_w)) {
//       vel.y = -1.0f;
//     } else if (shogle::engine_poll_key(shogle::key_s)) {
//       vel.y = 1.0f;
//     }
//     if (glm::length(vel) > 0) {
//       vel = speed*glm::normalize(vel);
//     }
//     _sprite->_vel = vel;
//   }
//
// public:
//   sprite_component* _sprite;
// };
//
// player::player(shogle::sprite* sprite) {
//   auto* spr = components.emplace(sprite_component{sprite, color4{1.0f}, 0});
//   spr->_transform.set_scale(spr->_sprite->corrected_scale()*50.0f);
//   components.emplace(movement_component{spr});
// }
//
// } // namespace ntf::game

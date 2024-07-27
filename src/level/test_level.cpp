// #include "level/test_level.hpp"
//
// #include "state.hpp"
//
// #include <shogle/core/log.hpp>
// #include <shogle/math/collision.hpp>
// #include <shogle/engine.hpp>
// #include <random>
// #include <coroutine>
//
// namespace ntf::game {
// static entity_movement player_mover;
//
//
// test_level::test_level() {
//   const auto vp = state.level_info.viewport->size();
//   auto& player = state.level_info.player;
//   player.transf().set_pos(vp.x*0.5f, vp.y*0.75f);
//
//   const cmplx vpc = cmplx{vp.x, vp.y}*0.5f;
//   auto dt = 1/60.0f;
//
//   player_mover = move_interpolate(400.0f*dt, 400.0f*dt*-I, .98);
//
//   // player_mover = move_asymptotic_simple(100.0f*dt, 5);
//   // player_mover = move_asymptotic_halflife(500.0f*dt, 400.0f*-I*dt, 15);
//   // player_mover = move_asymptotic(500.0f*dt, 50.0f*-I*dt, .8);
//   // player_mover = move_linear(100.0f*dt);
//   // player_mover = move_towards(vpc, dt, dt);
// }
//
// void test_level::render() {
//   auto& player = state.level_info.player;
//
//   for (auto& enemy : _enemies) {
//     enemy->render();
//   }
//   player.render();
//
//   for (auto& bullet : _danmaku) {
//     bullet->render();
//   }
//   player.render_hitbox();
// }
//
// void test_level::tick(float dt) {
//   auto& player = state.level_info.player;
//
//   tasker.tick(dt);
//
//   for (auto& enemy : _enemies) {
//     enemy->tick(_danmaku, dt);
//   }
//
//   auto pos = player.transf().cpos();
//   player_mover(pos);
//   player.transf().set_pos(pos);
//   player.tick(dt);
//
//   for (auto& bullet : _danmaku) {
//     bullet->tick(_danmaku, dt);
//   }
//
//   std::erase_if(_enemies, [](const auto& enemy) { return enemy->expired; });
//   std::erase_if(_danmaku, [](const auto& bullet) { return bullet->expired; });
// }
//
// void test_level::emplace_danmaku(uptr<renderable> bullet) {
//   _danmaku.emplace_back(std::move(bullet));
// }
//
// } // namespace ntf::game

// #pragma once
//
// #include "component/component.hpp"
//
// namespace ntf::game {
//
// class entity {
// public:
//   virtual ~entity() = default;
//
//   virtual void on_tick(float dt) {
//     for (auto& c : components) {
//       c->on_tick(dt);
//     }
//   }
//
//   virtual void on_render(float dt, float alpha) {
//     for (auto& c : components) {
//       c->on_render(dt, alpha);
//     }
//   }
//
//   virtual void on_update(float dt) {
//     for (auto& c : components) {
//       c->on_update(dt);
//     }
//   }
//
//   virtual void on_input(shogle::keycode code, shogle::keystate state) {
//     for (auto& c : components) {
//       c->on_input(code, state);
//     }
//   }
//
// protected:
//   component_vec components;
// };
//
// } // namespace ntf::game

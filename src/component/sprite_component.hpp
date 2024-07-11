// #pragma once
//
// #include "component/component.hpp"
//
// #include <shogle/scene/transform.hpp>
// #include <shogle/res/spritesheet.hpp>
//
// namespace ntf::game {
//
// class sprite_component : public component {
// public:
//   sprite_component(shogle::sprite* sprite, color4 color, size_t index) :
//     _sprite(sprite), _color(color), _index(index) {}
//
// public:
//   void on_render(float dt, float alpha) override;
//   void on_tick(float dt) override;
//
// public:
//   shogle::sprite* _sprite;
//   color4 _color;
//   size_t _index;
//   shogle::transform2d _transform{};
//   vec2 _vel{};
// };
//
// } // namespace ntf::game

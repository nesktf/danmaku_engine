// #pragma once
//
// #include "level/level.hpp"
//
// #include <shogle/core/task.hpp>
//
// namespace ntf::game {
//
// class test_level : public level {
// public:
//   test_level();
//
// public:
//   void render() override;
//   void tick(float dt) override;
//   void emplace_danmaku(uptr<renderable> bullet);
//
// private:
//   shogle::tasker tasker;
//   renderable::container _danmaku;
//   renderable::container _enemies;
//   uptr<renderable> _boss;
// };
//
// } // namespace ntf::game

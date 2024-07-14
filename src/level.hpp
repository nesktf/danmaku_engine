#pragma once

#include <shogle/core/task.hpp>
#include "enemy.hpp"
#include "boss.hpp"

namespace ntf::game {

struct level {
  virtual void render() = 0;
  virtual void tick(float dt) = 0;
};

class test_level : public level {
public:
  test_level();

public:
  void render() override;
  void tick(float dt) override;

public:
  std::vector<enemy> _enemies;
  shogle::tasker tasker;

  uptr<boss> _boss;
};

}

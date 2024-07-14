#include "level.hpp"

#include "render.hpp"
#include "state.hpp"

#include <shogle/core/log.hpp>

namespace ntf::game {

static shogle::sprite* fairy_sprite;
static auto fairy_fun(float t, cmplx dir) -> cmplx {
  return dir*cmplx{100.0f,100.0f*glm::cos(PI*t)};
}

class fairy_spawner : public shogle::tasker::task_type {
public:
  fairy_spawner(float T, std::vector<enemy>& enemies) : 
    _T(T), _enemies(enemies) {}

public:
  void tick(float dt) override {
    _t += dt;
    if (_t >= _T) {
      _enemies.emplace_back(fairy_sprite, fairy_fun, cmplx{0.0f}, math::expic(PI*0.5f));
      _t = 0.0f;
    }
  }

private:
  float _T;
  std::vector<enemy>& _enemies;

  float _t{};
};

test_level::test_level() {
  fairy_sprite = &resources.spritesheets.at("enemies")["fairy1"];
  auto& chen_sprite = resources.spritesheets.at("enemies")["chen1"];

  _boss = make_uptr<boss>(&chen_sprite, cmplx{0.0f, -40.0f});

  tasker.emplace_back(1.0f, [this]() { tasker.emplace_back<fairy_spawner>(1/2.0f, _enemies); });
}

void test_level::render() {
  for (auto& enemy : _enemies) {
    enemy.render();
  }
  _boss->render();
  state.player.render();
}

void test_level::tick(float dt) {
  tasker.tick(dt);

  for (auto& enemy : _enemies) {
    enemy.tick(dt, [](const auto& e) -> bool { 
      float limit = 250.0f;
      auto pos = e.pos();
      return (pos.x > limit || pos.x < -limit || pos.y > limit || pos.y < -limit);
    });
  }
  _boss->tick(dt, [](auto& enemy, float dt) {
    enemy._transf.set_rot(enemy._transf.rot() + PI*dt);
  });
  state.player.tick(dt);


  std::erase_if(_enemies, [](const auto& e) { return e.dead(); });
}

}

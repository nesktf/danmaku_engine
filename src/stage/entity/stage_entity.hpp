#include "global.hpp"
#include "resources.hpp"

#include "stage/entity/movement.hpp"

#include <shogle/scene/transform.hpp>

namespace stage {

class stage_entity {
public:
  stage_entity() = default;

public:
  void render();
  void tick();

protected:
  uint32_t lifetime;
  ntf::transform2d transform;

  res::atlas atlas;
  uint32_t atlas_index;
  uint32_t atlas_sequence;
  res::shader shader;
  std::function<void(stage_entity&)> custom_renderer;

  entity::movement movement;
  uint32_t collision_handler;
  std::function<void(stage_entity&)> custom_ticker; // ?
};

class boss_entity : public stage_entity {};

class item_entity : public stage_entity {};

class enemy_entity : public stage_entity {};

class bullet_entity : public stage_entity {};

} // namespace

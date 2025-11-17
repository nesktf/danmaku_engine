#pragma once

#include "../render/stage.hpp"
#include "../util/free_list.hpp"
#include "./entity.hpp"

namespace okuu::stage {

class stage_scene {
public:
  static constexpr size_t MAX_BOSSES = 4u;

public:
  template<typename T>
  using idx_elem = std::pair<u32, T>;

public:
  stage_scene(u32 max_entities, player_entity&& player_,
              std::vector<assets::sprite_atlas>&& atlas_assets_);

public:
  void tick();
  void render(double dt, double alpha);

public:
  ntf::optional<idx_elem<assets::sprite_atlas::sprite>> find_sprite(std::string_view name) const;
  ntf::optional<idx_elem<assets::sprite_atlas::animation>> find_anim(std::string_view name) const;

public:
  util::free_list<projectile_entity> projs;
  std::array<ntf::nullable<boss_entity>, MAX_BOSSES> bosses;
  u32 boss_count;
  player_entity player;
  std::vector<assets::sprite_atlas> atlas_assets;
  render::stage_renderer renderer;
  u32 task_wait_ticks, ticks;
};

} // namespace okuu::stage

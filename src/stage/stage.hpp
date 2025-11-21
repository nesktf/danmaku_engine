#pragma once

#include "./entity.hpp"

#include "../render/stage.hpp"
#include "../util/free_list.hpp"

namespace okuu::stage {

class stage_scene {
public:
  static constexpr size_t MAX_BOSSES = 4u;

public:
  stage_scene(player_entity&& player, render::stage_renderer&& renderer);

public:
  void tick();
  void render(double dt, double alpha, assets::asset_bundle& assets);

public:
  u64 spawn_projectile(projectile_args&& args);
  void kill_projectile(u64 handle);
  bool is_projectile_alive(u64 handle);
  void set_proj_pos(u64 handle, f32 x, f32 y);
  vec2 get_proj_pos(u64 handle);
  void set_proj_mov(u64 handle, stage::entity_movement movement);

  ntf::optional<u32> spawn_boss(const boss_args& args);
  void kill_boss(u32 slot);
  boss_entity& get_boss(u32 slot);

  player_entity& get_player() { return _player; }

public:
  void task_wait(u32 ticks) { _task_wait_ticks = ticks; }

  u32 task_wait() const { return _task_wait_ticks; }

private:
  render::stage_renderer _renderer;
  util::free_list<projectile_entity> _projs;
  std::array<boss_entity, MAX_BOSSES> _bosses;
  u32 _boss_count;
  player_entity _player;
  u32 _task_wait_ticks, _ticks;
};

} // namespace okuu::stage

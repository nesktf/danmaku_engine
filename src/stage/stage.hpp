#pragma once

#include "../lua/stage_env.hpp"

#include "../assets/manager.hpp"
#include "../render/stage.hpp"
#include "../util/free_list.hpp"
#include "./entity.hpp"

namespace okuu {

namespace stage {

class stage_scene {
public:
  static constexpr size_t MAX_BOSSES = 4u;

public:
  stage_scene(player_entity&& player, render::stage_renderer&& renderer);

public:
  void tick();
  void render(double dt, double alpha);

public:
  u64 spawn_projectile(const projectile_args& args);
  void kill_projectile(u64 handle);
  bool is_projectile_alive(u64 handle);

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

} // namespace stage

class game_state {
public:
  game_state(std::unique_ptr<assets::asset_bundle>&& assets,
             std::unique_ptr<stage::stage_scene>&& scene, lua::stage_env&& lua_env);

public:
  static expect<game_state> load_from_package(const std::string& path, chima::context& chima);

public:
  void tick();
  void render(f64 dt, f64 alpha);

private:
  std::unique_ptr<assets::asset_bundle> _assets;
  std::unique_ptr<stage::stage_scene> _scene;
  lua::stage_env _lua_env;
  f32 _t;
};

} // namespace okuu

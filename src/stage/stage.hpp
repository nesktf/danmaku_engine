#pragma once

#include "../render/stage.hpp"
#include "../util/free_list.hpp"
#include "./entity.hpp"

#define SOL_ALL_SAFETIES_ON 1
#include <sol/sol.hpp>

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

class lua_env {
public:
  class projectile_view {
  public:
    projectile_view(util::free_list<projectile_entity>& list, size_t count);

  public:
    void for_each(sol::function f);

  public:
    size_t size() const { return _items.size(); }

  private:
    std::vector<u32> _items;
    ntf::weak_ptr<util::free_list<projectile_entity>> _list;
  };

public:
  lua_env(sol::state&& lua, sol::table lib_table, std::unique_ptr<stage_scene>&& scene);

public:
  static expect<lua_env> load(const std::string& script_path,
                              std::unique_ptr<stage_scene>&& scene);

public:
  void tick();

  stage_scene& scene() { return *_scene; }

  const stage_scene& scene() const { return *_scene; }

private:
  sol::state _lua;

  sol::thread _task_thread;
  sol::coroutine _task;

  u32 _ticks, _frames;
  u32 _task_time;

  std::unique_ptr<stage_scene> _scene;
};

} // namespace okuu::stage

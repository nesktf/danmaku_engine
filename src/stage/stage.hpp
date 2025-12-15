#pragma once

#include "./entity.hpp"

#include "../render/stage.hpp"

namespace okuu::stage {

// template<typename T>
// concept entity_type = requires(T entity, const T const_entity, typename T::args_type args) {
//   { const_entity.pos(real{}, real{}) } -> std::same_as<vec2>;
//   { entity.set_pos(real{}, real{}) } -> ntf::meta::same_as_any<T&, void>;
//   { const_entity.sprite() } -> std::same_as<entity_sprite>;
//   requires(std::constructible_from<T, typename T::args_type>);
// };

template<typename T>
class entity_list {
public:
  using args_type = typename T::args_type;
  using entity_handle = u64;

public:
  template<typename... Args>
  entity_handle spawn(Args&&... args) {
    auto elem = _entities.emplace(std::forward<Args>(args)...);
    return elem.as_u64();
  }

  void kill(entity_handle handle) { _entities.remove(ntf::freelist_handle::from_u64(handle)); }

  bool is_alive(entity_handle handle) {
    return _entities.is_valid(ntf::freelist_handle::from_u64(handle));
  }

  T& at(entity_handle handle) { return _entities[ntf::freelist_handle::from_u64(handle)]; }

  const T& at(entity_handle handle) const {
    return _entities[ntf::freelist_handle::from_u64(handle)];
  }

  template<typename F>
  void for_each(F&& func) {
    for (auto& [ent, _] : _entities) {
      std::invoke(func, ent);
    }
  }

  template<typename F>
  void for_each(F&& func) const {
    for (const auto& [ent, _] : _entities) {
      std::invoke(func, ent);
    }
  }

  template<typename F>
  void clear_where(F&& func) {
    _entities.remove_where(std::forward<F>(func));
  }

private:
  ntf::freelist<T> _entities;
};

class stage_scene {
public:
  static constexpr size_t MAX_BOSSES = 4u;

public:
  stage_scene(player_entity&& player, render::stage_renderer&& renderer);

public:
  void tick();
  void render(double dt, double alpha, assets::asset_bundle& assets);

public:
  entity_list<projectile_entity>& get_projectiles() { return _projs; }

  entity_list<sprite_entity>& get_sprites() { return _sprites; }

  ntf::optional<u32> spawn_boss(const boss_args& args);
  void kill_boss(u32 slot);
  boss_entity& get_boss(u32 slot);

  player_entity& get_player() { return _player; }

public:
  void task_wait(u32 ticks) { _task_wait_ticks = ticks; }

  u32 task_wait() const { return _task_wait_ticks; }

private:
  render::stage_renderer _renderer;
  entity_list<projectile_entity> _projs;
  entity_list<sprite_entity> _sprites;
  std::array<boss_entity, MAX_BOSSES> _bosses;
  u32 _boss_count;
  player_entity _player;
  u32 _task_wait_ticks, _ticks;
};

} // namespace okuu::stage

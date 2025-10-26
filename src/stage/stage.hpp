#pragma once

#include "../render/stage.hpp"
#include "./entity.hpp"

#define SOL_ALL_SAFETIES_ON 1
#include <sol/sol.hpp>

namespace okuu::stage {

template<typename T>
using expect = ntf::expected<T, std::string>;

template<typename T>
class free_list {
public:
  class element {
  public:
    element(free_list& list, u32 idx) : _list{list}, _idx{idx} {}

  public:
    T& operator*() {
      NTF_ASSERT(_idx < _list->_elems.size());
      auto& proj = _list->_elems[_idx];
      NTF_ASSERT(proj.has_value());
      return *proj;
    }

    const T& operator*() const {
      NTF_ASSERT(_idx < _list->_elems.size());
      auto& proj = _list->_elems[_idx];
      NTF_ASSERT(proj.has_value());
      return *proj;
    }

    bool alive() const {
      NTF_ASSERT(_idx < _list->_elems.size());
      auto& proj = _list->_elems[_idx];
      return proj.has_value();
    }

    T* operator->() { return &**this; }

    const T* operator->() const { return &**this; }

    u32 idx() const { return _idx; }

  private:
    ntf::weak_ptr<free_list> _list;
    u32 _idx;

    friend class free_list;
  };

  friend class element;

public:
  free_list();

public:
  template<typename... Args>
  element request_elem(Args&&... args) {
    if (!_free.empty()) {
      const u32 pos = _free.back();
      _free.pop();
      auto& elem = _elems[pos];
      elem.emplace(std::forward<Args>(args)...);
      return {*this, pos};
    }

    _elems.emplace(std::in_place, std::forward<Args>(args)...);
    return {*this, static_cast<u32>(_elems.size()) - 1u};
  }

  void return_elem(element elem) {
    const u32 idx = elem._idx;
    NTF_ASSERT(idx < _elems.size());
    auto& proj = _elems[idx];
    NTF_ASSERT(proj.has_value());
    proj.reset();
    _free.push(idx);
  }

  void clear() {
    _elems.clear();
    while (!_free.empty()) {
      _free.pop();
    }
  }

  ntf::span<ntf::nullable<T>> elems() { return {_elems.begin(), _elems.size()}; }

  ntf::cspan<ntf::nullable<T>> elems() const { return {_elems.begin(), _elems.size()}; }

private:
  std::vector<ntf::nullable<T>> _elems;
  std::queue<u32> _free;
};

class stage_scene {
public:
  static constexpr size_t MAX_BOSSES = 4u;

public:
  template<typename T>
  using idx_elem = std::pair<u32, T>;

public:
  stage_scene(player_entity&& player_, render::stage_viewport&& viewport) :
      projs{}, bosses{}, boss_count{0u}, player{std::move(player_)},
      _viewport{std::move(viewport)} {}

public:
  ntf::optional<idx_elem<assets::sprite_atlas::sprite>> find_sprite(std::string_view name) const {
    for (u32 i = 0; const auto& curr : atlas_assets) {
      auto item = curr.find_sprite(name);
      if (item) {
        return {ntf::in_place, i, *item};
      }
      ++i;
    }
    return {ntf::nullopt};
  }

  ntf::optional<idx_elem<assets::sprite_atlas::animation>> find_anim(std::string_view name) const {
    for (u32 i = 0; const auto& curr : atlas_assets) {
      auto item = curr.find_animation(name);
      if (item) {
        return {ntf::in_place, i, *item};
      }
      ++i;
    }
    return {ntf::nullopt};
  }

  // The scene has to render the following (in order):
  // - The background
  // - The boss(es)
  // - The player
  // - The items
  // - The danmaku
  void render(double dt, double alpha);

public:
  free_list<projectile_entity> projs;
  std::array<ntf::nullable<boss_entity>, MAX_BOSSES> bosses;
  size_t boss_count;
  player_entity player;
  std::vector<assets::sprite_atlas> atlas_assets;
  render::stage_viewport _viewport;
  u32 wait_time;
};

class lua_env {
public:
  class projectile_view {
  public:
    projectile_view(free_list<projectile_entity>& list, size_t count);

  public:
    void for_each(sol::function f);

  public:
    size_t size() const { return _items.size(); }

  private:
    std::vector<u32> _items;
    ntf::weak_ptr<free_list<projectile_entity>> _list;
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
  u32 _task_time, _task_wait;

  std::unique_ptr<stage_scene> _scene;
};

} // namespace okuu::stage

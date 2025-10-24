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

    T* operator->() { return &**this; }

    const T* operator->() const { return &**this; }

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

class scene {
public:
  static constexpr size_t MAX_BOSSES = 4u;

  struct scene_objects {
    scene_objects(player_entity&& player_) :
        projs{}, bosses{}, boss_count{0u}, player{std::move(player_)} {}

    free_list<projectile_entity> projs;
    std::array<ntf::nullable<boss_entity>, MAX_BOSSES> bosses;
    size_t boss_count;
    player_entity player;
  };

public:
  scene(sol::state&& lua, sol::table lib_table, std::unique_ptr<scene_objects>&& objs,
        render::stage_viewport&& vp);

public:
  static expect<scene> load(const std::string& script_path);

public:
  void tick();

  // The scene has to render the following (in order):
  // - The background
  // - The boss(es)
  // - The player
  // - The items
  // - The danmaku
  void render(double dt, double alpha);

private:
  sol::state _lua;

  sol::thread _task_thread;
  sol::coroutine _task;

  u32 _ticks, _frames;
  u32 _task_time, _task_wait;

  std::unique_ptr<scene_objects> _objects;
  render::stage_viewport _viewport;
};

} // namespace okuu::stage

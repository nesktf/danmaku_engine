#pragma once

#include <sol/sol.hpp>

#include "global.hpp"
#include "render.hpp"

#include "stage/entity.hpp"
#include <list>

namespace stage {

template<typename T>
using entity_list = std::list<T>;

template<typename T, typename Allocator>
class entity_pool {
public:
  using allocator_type = Allocator;

private:
  struct pool_header {
    pool_header* next;
  };

public:
  entity_pool() = default;

public:
  T* acquire();
  void release(T* obj);

public:
  std::size_t used() const { return _used; }
  std::size_t allocated() const { return _allocated; }

private:
  allocator_type _allocator;
  pool_header* _free{nullptr};
  std::size_t _allocated{0}, _used{0};
};

class context {
public:
  context(std::string_view stage_script);

public:
  void tick();
  void render(double dt, [[maybe_unused]] double alpha);

public:
  std::list<stage::projectile>& projs() { return _projs; }

private:
  void _prepare_player();
  void _prepare_lua_env();

private:
  sol::state _lua;
  sol::protected_function _entrypoint;

  frames _tick_count{0}, _frame_count{0};
  frames _task_time{0}, _task_wait{0};

  std::list<stage::projectile> _projs;
  stage::boss _boss;
  stage::player _player;

  render::stage_viewport _viewport;

  render::viewport_event::subscription _vp_sub;

public:
  ~context() noexcept;
  NTF_DISABLE_COPY(context);
};

class projectile_view {
public:
  using iterator = stage::entity_list<stage::projectile>::iterator;

public:
  projectile_view(std::size_t size);

public:
  void for_each(sol::function f);

public:
  std::size_t size() const { return _size; }

private:
  iterator _begin;
  std::size_t _size;
};

} // namespace stage


namespace stage {

template<typename T, typename Allocator>
auto entity_pool<T, Allocator>::acquire() -> T* {
  pool_header* header = _free;
  T* obj;

  if (_free) {
    obj = reinterpret_cast<T*>(_free);
    _free = _free->next;
  } else {
    obj = _allocator.allocate(1);
    ++_allocated;
  }

  _allocator.construct(obj, T{});
  ++_used;

  return obj;
}

template<typename T, typename Allocator>
void entity_pool<T, Allocator>::release(T* obj) {
  _allocator.destroy(obj);
  pool_header* header = reinterpret_cast<pool_header*>(obj);
  header->next = _free;
  _free = header;
  --_used;
}

} // namespace stage

#pragma once

#include "global.hpp"

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

void init();
void destroy();

void load(std::string_view stage_script);

void tick();
void render(double dt, [[maybe_unused]] double alpha);

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

#pragma once

#include "global.hpp"

#include <shogle/core/allocator.hpp>

namespace stage {

void* arena_allocate(size_t sz);
void arena_free(void* ptr);

struct arena_wrapper {
  void* allocate(size_t sz) { return arena_allocate(sz); }
  void deallocate(void* ptr) { arena_free(ptr); }
};

template<typename T>
using arena_allocator = ntf::allocator_adapter<T, arena_wrapper>;


template<typename T, typename Allocator = arena_allocator<T>>
class entity_pool {
private:
  struct pool_header {
    pool_header* next;
  };

public:
  using allocator_type = Allocator;

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

} // namespace stage

#ifndef STAGE_ENTITY_POOL_INL
#include "stage/entity_pool.inl"
#endif

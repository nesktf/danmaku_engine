#define STAGE_ENTITY_POOL_INL
#include "stage/entity_pool.hpp"
#undef STAGE_ENTITY_POOL_INL

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

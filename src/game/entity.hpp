#pragma once

#include "global.hpp"
#include "resources.hpp"
#include "math.hpp"

#include <shogle/scene/transform.hpp>

namespace game::entity {

template<typename T>
concept EntityType = requires(T entity) {
  { entity.sprite() } -> std::convertible_to<res::sprite>;
  { entity.mat() } -> std::convertible_to<mat4>;
};


class movement {
public:
  void tick(ntf::transform2d& transform);

public:
  cmplx vel{}, acc{};
  real ret{};

  cmplx attr{}, attr_p{};
  real attr_exp{};
};

movement movement_linear(cmplx vel);
movement movement_interp(cmplx vel0, cmplx vel1, real ret);
movement movement_interp_hl(cmplx vel0, cmplx vel1, real hl);
movement movement_interp_simple(cmplx vel, real boost);
movement movement_towards(cmplx target, cmplx vel, cmplx attr, real ret);


class animator {
public:
  void tick(frames entity_ticks);

public:
  res::sprite sprite() const;

public:
  res::atlas handle;
  res::atlas_entry index{};
  res::atlas_sequence sequence{};
  bool use_sequence {false};
};

animator animator_static(res::sprite sprite);
animator animator_sequence(res::sequence_pair sequence);


class projectile {
public:
  using movement_type = movement;
  using animator_type = animator;

  struct args {
    ntf::transform2d transform{};
    movement_type movement;
    animator_type animator;
    real angular_speed {0.f};
    bool clean_flag {true};
  };

public:
  projectile() :
    _birth(global::state().elapsed_ticks) {}

  projectile(args arg) :
    transform(arg.transform),
    movement(arg.movement),
    animator(arg.animator),
    angular_speed(arg.angular_speed),
    clean_flag(arg.clean_flag),
    _birth(global::state().elapsed_ticks) {}

public:
  void tick();

public:
  frames birth() const { return _birth; }
  frames lifetime() const { return _lifetime; }
  res::sprite sprite() const { return animator.sprite(); }
  const mat4& mat() { return transform.mat(); }

public:
  ntf::transform2d transform;

  movement_type movement;
  animator_type animator;

  real angular_speed;

  bool clean_flag;

private:
  frames _birth;
  frames _lifetime{0};
};


class boss {
public:
  using movement_type = movement;
  using animator_type = animator;

  struct args {
    ntf::transform2d transform{};
    movement_type movement{};
    animator_type animator{};
  };

public:
  boss() :
    _birth(global::state().elapsed_ticks) {}

  boss(args arg) :
    transform(arg.transform),
    movement(arg.movement),
    animator(arg.animator),
    hide(false),
    _birth(global::state().elapsed_ticks) {}

public:
  void tick();

public:
  frames birth() const { return _birth; }
  frames lifetime() const { return _lifetime; }
  res::sprite sprite() const { return animator.sprite(); }
  const mat4& mat() { return transform.mat(); }

public:
  ntf::transform2d transform;

  movement_type movement;
  animator_type animator;

  bool hide {true};

private:
  frames _birth;
  frames _lifetime{0};
};


class player {
public:
  class movement_type {
  public:
    movement_type() = default;
    movement_type(real base, real slow) :
      base_speed(base), slow_speed(slow) {}

  public:
    void tick(ntf::transform2d& transform);

  public:
    cmplx vel() const { return _vel; }
   
  private:
    cmplx _vel;

  public:
    real base_speed{1.f};
    real slow_speed{1.f};
  };

  class animator_type {
  public:
    enum animation_state : uint8_t {
      IDLE = 0,
      LEFT,
      LEFT_TO_IDLE,
      IDLE_TO_LEFT,
      RIGHT,
      RIGHT_TO_IDLE,
      IDLE_TO_RIGHT,
      ANIM_COUNT,
    };
    using animation_data = std::array<res::atlas_type::sequence_handle, ANIM_COUNT>;

  public:
    animator_type() = default;
    animator_type(res::atlas atlas, animation_data data) { set_data(atlas, data); }

  public:
    void tick(const movement_type& movement);
    void set_data(res::atlas atlas, animation_data data);
    
    res::sprite sprite() const;

  private:
    res::sprite_animator _animator;
    animation_data _anim;
    animation_state _state;
  };

  struct args {
    ntf::transform2d transform;
    res::atlas atlas;
    animator_type::animation_data anim;
    real base_speed{1.f};
    real slow_speed{1.f};
  };

public:
  player() = default;

  player(args arg) :
    transform(arg.transform),
    movement(arg.base_speed, arg.slow_speed),
    animator(arg.atlas, arg.anim) {}

public:
  void tick();

public:
  res::sprite sprite() const { return animator.sprite(); }
  const mat4& mat() { return transform.mat(); }

public:
  ntf::transform2d transform;

  movement_type movement;
  animator_type animator;
};

template<EntityType T, typename Allocator>
class pool {
public:
  using allocator_type = Allocator;

private:
  struct pool_header {
    pool_header* next;
  };

public:
  pool() = default;

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

} // namespace game::entity


namespace game::entity {

template<EntityType T, typename Allocator>
auto pool<T, Allocator>::acquire() -> T* {
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

template<EntityType T, typename Allocator>
void pool<T, Allocator>::release(T* obj) {
  _allocator.destroy(obj);
  pool_header* header = reinterpret_cast<pool_header*>(obj);
  header->next = _free;
  _free = header;
  --_used;
}

} // namespace game::entity

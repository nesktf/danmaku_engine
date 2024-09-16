#pragma once

#include "global.hpp"
#include "resources.hpp"
#include "math.hpp"

#include <shogle/scene/transform.hpp>

namespace stage {

template<typename T>
concept entity_type = requires(T entity) {
  { entity.sprite() } -> std::convertible_to<res::sprite>;
  { entity.mat() } -> std::convertible_to<mat4>;
};

class entity_movement {
public:
  void tick(ntf::transform2d& transform);

public:
  cmplx vel{}, acc{};
  real ret{};

  cmplx attr{}, attr_p{};
  real attr_exp{};
};

class entity_animator {
public:
  void tick(frames entity_ticks);

public:
  res::sprite sprite() const;

public:
  res::atlas handle;
  res::atlas_type::texture_handle index{};
  res::atlas_type::sequence_handle sequence{};
  bool use_sequence {false};
};

entity_animator entity_animator_static(res::atlas atlas, res::atlas_type::texture_handle index);
entity_animator entity_animator_sequence(res::atlas atlas, res::atlas_type::sequence_handle seq);

entity_movement entity_movement_linear(cmplx vel);
entity_movement entity_movement_interp(cmplx vel0, cmplx vel1, real ret);
entity_movement entity_movement_interp_hl(cmplx vel0, cmplx vel1, real hl);
entity_movement entity_movement_interp_simple(cmplx vel, real boost);
entity_movement entity_movement_towards(cmplx target, cmplx vel, cmplx attr, real ret);

} // namespace stage

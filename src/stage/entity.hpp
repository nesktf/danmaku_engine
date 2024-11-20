#pragma once

#include "okuu.hpp"
#include "resources.hpp"

namespace okuu {

class entity_movement {
public:
  void tick(ntf::transform2d& transform);

public:
  cmplx vel{}, acc{};
  real ret{};

  cmplx attr{}, attr_p{};
  real attr_exp{};
};

okuu::entity_movement movement_linear(cmplx vel);
okuu::entity_movement movement_interp(cmplx vel0, cmplx vel1, real ret);
okuu::entity_movement movement_interp_hl(cmplx vel0, cmplx vel1, real hl);
okuu::entity_movement movement_interp_simple(cmplx vel, real boost);
okuu::entity_movement movement_towards(cmplx target, cmplx vel, cmplx attr, real ret);


class entity_animator {
public:
  void tick(frames entity_ticks);

public:
  okuu::sprite sprite() const;

private:
  okuu::resource<okuu::atlas> _atlas;
  okuu::atlas_texture _tex{ntf::atlas_tombstone};
  okuu::atlas_sequence _seq{ntf::atlas_sequence_tombstone};
};

okuu::entity_animator animator_static(okuu::sprite sprite);
okuu::entity_animator animator_sequence(okuu::sprite_sequence sequence);
//
//
// class entity {
// public:
//   entity(frames birth, ntf::transform2d transf = {}, res::sprite sprite = {}) :
//     _transform(std::move(transf)), _sprite(sprite), _birth(birth) {}
//
// public:
//   void transform(ntf::transform2d transf) { _transform = std::move(transf);}
//   void sprite(res::sprite sprite) { _sprite = sprite; }
//
// public:
//   frames birth() const { return _birth; }
//   frames lifetime() const { return _lifetime; }
//
//   const ntf::transform2d& transform() const { return _transform; }
//
//   const mat4& tmat() { return _transform.mat(); }
//   res::sprite sprite() const { return _sprite; }
//
// protected:
//   void _tick_time() { ++_lifetime; }
//
// protected:
//   ntf::transform2d _transform;
//   res::sprite _sprite;
//
// private:
//   frames _birth;
//   frames _lifetime{0};
// };
//
} // namespace okuu

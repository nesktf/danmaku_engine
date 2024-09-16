#include "stage/entity.hpp"

namespace stage {

void entity_movement::tick(ntf::transform2d& transform) {
  // const cmplx v0 = vel;
  cmplx pos = transform.cpos();

  pos += vel;
  vel = acc + ret*vel;

  if (attr != cmplx{}) {
    const cmplx av = attr_p - pos;
    if (attr_exp == 1) {
      vel += attr*av;
    } else {
      real norm2 = math::norm2(av);
      norm2 = std::pow(norm2, attr_exp - .5f);
      vel += attr + (av*norm2);
    }
  }

  transform.set_pos(pos);
}


void entity_animator::tick(frames entity_ticks) {
  if (!use_sequence) {
    return;
  }

  const auto& seq = handle->sequence_at(sequence);
  index = seq[entity_ticks%seq.size()];
}

res::sprite entity_animator::sprite() const {
  return res::sprite{handle, index};
}


entity_animator entity_animator_static(res::atlas atlas,
                                       res::atlas_type::texture_handle index) {
  return entity_animator {
    .handle = atlas,
    .index = index,
    .use_sequence = false,
  };
}

entity_animator entity_animator_sequence(res::atlas atlas,
                                         res::atlas_type::sequence_handle seq) {
  return entity_animator {
    .handle = atlas,
    .sequence = seq,
    .use_sequence = true,
  };
}

entity_movement entity_movement_linear(cmplx vel) {
  return entity_movement {
    .vel = vel,
    .acc = 0,
    .ret = 1,
  };
}

entity_movement entity_movement_interp(cmplx vel0, cmplx vel1, real ret) {
  return entity_movement {
    .vel = vel0,
    .acc = vel1*(1-ret),
    .ret = ret,
  };
}

entity_movement entity_movement_interp_hl(cmplx vel0, cmplx vel1, real hl) {
  return entity_movement_interp(vel0, vel1, std::exp2(-1.f/hl));
}

entity_movement entity_movement_interp_simple(cmplx vel, real boost) {
  return entity_movement_interp(vel*(1+boost), vel, .8f);
}

entity_movement entity_movement_towards(cmplx target, cmplx vel, cmplx attr,
                                        real ret) {
  return entity_movement {
    .vel = vel,
    .ret = ret,
    .attr = attr,
    .attr_p = target,
    .attr_exp = 1,
  };
}

} // namespace stage

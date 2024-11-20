#include "stage/movement.hpp"

namespace okuu::stage {

void movement::tick(ntf::transform2d& transform) {
  // const cmplx v0 = vel;
  cmplx pos = transform.cpos();

  pos += vel;
  vel = acc + ret*vel;

  if (attr != cmplx{}) {
    const cmplx av = attr_p - pos;
    if (attr_exp == 1) {
      vel += attr*av;
    } else {
      real norm2 = okuu::norm2(av);
      norm2 = std::pow(norm2, attr_exp - .5f);
      vel += attr + (av*norm2);
    }
  }

  transform.pos(pos);
}


movement movement_linear(cmplx vel) {
  return movement {
    .vel = vel,
    .acc = 0,
    .ret = 1,
  };
}

movement movement_interp(cmplx vel0, cmplx vel1, real ret) {
  return movement {
    .vel = vel0,
    .acc = vel1*(1-ret),
    .ret = ret,
  };
}

movement movement_interp_hl(cmplx vel0, cmplx vel1, real hl) {
  return movement_interp(vel0, vel1, std::exp2(-1.f/hl));
}

movement movement_interp_simple(cmplx vel, real boost) {
  return movement_interp(vel*(1+boost), vel, .8f);
}

movement movement_towards(cmplx target, cmplx vel, cmplx attr, real ret) {
  return movement {
    .vel = vel,
    .ret = ret,
    .attr = attr,
    .attr_p = target,
    .attr_exp = 1,
  };
}

} // namespace okuu::stage

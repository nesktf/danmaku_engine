#pragma once

#include "math.hpp"

namespace ntf {

class entity_movement {
public:
  cmplx operator()(cmplx& pos) {
    const auto v0 = velocity;
    pos += v0;
    velocity = acceleration + retention*velocity;

    if (attraction != cmplx{}) {
      const auto av = attraction_point - pos;
      if (attraction_exponent == 1) {
        velocity += attraction*av;
      } else {
        float norm2 = av.real()*av.real() + av.imag()*av.imag();
        norm2 = std::pow(norm2, attraction_exponent - 0.5);
        velocity += attraction + (av*norm2);
      }
    }
    return velocity;
  }

  cmplx operator()(cmplx& pos, uint times) {
    const auto v0 = velocity;
    
    for (uint i = 0; i < times; ++i) {
      (*this)(pos);
    }

    return v0;
  }

public:
  cmplx velocity{}, acceleration{};
  float retention{};

  cmplx attraction{}, attraction_point{};
  float attraction_exponent{};
};

inline entity_movement move_linear(cmplx vel) {
  return entity_movement {
    .velocity = vel,
    .acceleration = 0,
    .retention = 1,
  };
}

inline entity_movement move_towards(cmplx target, cmplx vel, cmplx att) {
  return entity_movement {
    .velocity = vel,
    .attraction = att,
    .attraction_point = target,
    .attraction_exponent = 1
  };
}

inline entity_movement move_interpolate(cmplx vel0, cmplx vel1, float ret) {
  return entity_movement {
    .velocity = vel0,
    .acceleration = vel1*(1-ret),
    .retention = ret,
  };
}

inline entity_movement move_interpolate_halflife(cmplx vel0, cmplx vel1, float hl) {
  return move_interpolate(vel0, vel1, std::exp2(-1.f/hl));
}

inline entity_movement move_interpolate_simple(cmplx vel, float boost) {
  return move_interpolate(vel*(1+boost), vel, 0.8f);
}

} // namespace ntf

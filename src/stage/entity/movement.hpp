#pragma once

#include "math.hpp"

namespace entity {

class movement {
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
        float norm2 = math::norm2(av);
        norm2 = std::pow(norm2, attraction_exponent - 0.5);
        velocity += attraction + (av*norm2);
      }
    }
    return v0;
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

inline movement move_linear(cmplx vel) {
  return movement {
    .velocity = vel,
    .acceleration = 0,
    .retention = 1,
  };
}

inline movement move_towards(cmplx target, cmplx vel, cmplx att, float ret) {
  return movement {
    .velocity = vel,
    .retention = ret,
    .attraction = att,
    .attraction_point = target,
    .attraction_exponent = 1
  };
}

inline movement move_interpolate(cmplx vel0, cmplx vel1, float ret) {
  return movement {
    .velocity = vel0,
    .acceleration = vel1*(1-ret),
    .retention = ret,
  };
}

inline movement move_interpolate_halflife(cmplx vel0, cmplx vel1, float hl) {
  return move_interpolate(vel0, vel1, std::exp2(-1.f/hl));
}

inline movement move_interpolate_simple(cmplx vel, float boost) {
  return move_interpolate(vel*(1+boost), vel, 0.8f);
}

} // namespace entity

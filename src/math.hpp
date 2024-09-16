#pragma once

#include "global.hpp"

namespace math {

constexpr real norm2(cmplx z) {
  return z.real()*z.real() + z.imag()*z.imag();
}

constexpr real norm2(vec2 v) {
  return v.x*v.x + v.y*v.y;
}

constexpr real norm(cmplx z) {
  return glm::sqrt(norm2(z));
}

constexpr real norm(vec2 v) {
  return glm::sqrt(norm2(v));
}

constexpr cmplx normalize(cmplx z) {
  return z/glm::sqrt(norm2(z));
}

constexpr vec2 normalize(vec2 v) {
  return v/glm::sqrt(norm2(v));
}

constexpr cmplx conv(const vec2& v) {
  return cmplx{v.x, v.y};
}

constexpr vec2 conv(const cmplx& z) {
  return vec2{z.real(), z.imag()};
}

} // namespace math

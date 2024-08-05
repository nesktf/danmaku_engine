#pragma once

#include "core.hpp"

namespace math {

constexpr float norm2(cmplx z) {
  return z.real()*z.real() + z.imag()*z.imag();
}

constexpr float norm(cmplx z) {
  return glm::sqrt(norm2(z));
}

constexpr cmplx normalize(cmplx z) {
  return z/glm::sqrt(norm2(z));
}

constexpr vec2 conv(const cmplx& z) {
  return vec2{z.real(), z.imag()};
}

constexpr cmplx conv(const vec2& v) {
  return cmplx{v.x, v.y};
}

} // namespace math

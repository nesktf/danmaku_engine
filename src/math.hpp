#pragma once

#include "core.hpp"

namespace math {

constexpr float cnorm2(cmplx z) {
  return z.real()*z.real() + z.imag()*z.imag();
}

constexpr cmplx cnormalize(cmplx z) {
  return z/glm::sqrt(cnorm2(z));
}

} // namespace math

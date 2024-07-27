#pragma once

#include <shogle/core/types.hpp>

namespace ntf::game {

constexpr float cnorm2(cmplx z) {
  return z.real()*z.real() + z.imag()*z.imag();
}

constexpr cmplx cnormalize(cmplx z) {
  return z/glm::sqrt(cnorm2(z));
}

} // namespace ntf::game

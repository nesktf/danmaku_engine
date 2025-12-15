#pragma once

#include <ntfstl/expected.hpp>
#include <ntfstl/freelist.hpp>
#include <ntfstl/logger.hpp>
#include <ntfstl/types.hpp>
#include <shogle/math/vector.hpp>

#define fn auto

namespace okuu {

template<typename T>
using expect = ntf::expected<T, std::string>;

using namespace ntf::numdefs;

using logger = ntf::logger;

using real = f32;
using shogle::cmplx;
using shogle::ivec2;
using shogle::mat3;
using shogle::mat4;
using shogle::uvec2;
using shogle::vec2;
using shogle::vec3;
using shogle::vec4;

using color4 = vec4;

static constexpr u32 GAME_UPS = 60;

constexpr u32 secs_to_ticks(f32 secs) noexcept {
  return static_cast<u32>(std::floor(secs * okuu::GAME_UPS));
}

} // namespace okuu

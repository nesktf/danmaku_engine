#pragma once

#include <shogle/math/alg.hpp>

#include <shogle/engine.hpp>

using ntf::vec2;
using ntf::vec3;
using ntf::vec4;
using ntf::ivec2;
using ntf::cmplx;

using ntf::mat3;
using ntf::mat4;

using ntf::color3;
using ntf::color4;

using frames = uint32_t;
using frame_delay = int32_t;

using renderer = ntf::gl;

const constexpr uint UPS = 60;
const constexpr float DT = 1.f/UPS;
const constexpr ivec2 WIN_SIZE {1280, 720};
// const constexpr ivec2 VIEWPORT = {880, 660};
const constexpr ivec2 VIEWPORT {600, 700};
const constexpr auto fb_ratio = 6.f/7.f;

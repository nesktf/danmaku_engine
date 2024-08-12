#pragma once

#include <shogle/core/types.hpp>

using ntf::vec2;
using ntf::vec3;
using ntf::vec4;
using ntf::ivec2;
using ntf::cmplx;

using ntf::mat3;
using ntf::mat4;

using ntf::color3;
using ntf::color4;

using ntf::strmap;
using std::vector;

const constexpr uint UPS = 60;
const constexpr float DT = 1.f/UPS;
const constexpr ivec2 WIN_SIZE {1280, 720};
const constexpr ivec2 VIEWPORT = {880, 660};
// const constexpr ivec2 VIEWPORT {768, 896};
const constexpr auto fb_ratio = 6.f/7.f;

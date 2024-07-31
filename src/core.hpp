#pragma once

#include <shogle/engine.hpp>
#include <shogle/core/types.hpp>
#include <shogle/scene/transform.hpp>
#include <shogle/res/spritesheet.hpp>
#include <shogle/render/shader.hpp>
#include <shogle/res/spritesheet.hpp>
#include <shogle/res/font.hpp>
#include <shogle/render/framebuffer.hpp>
#include <shogle/scene/camera.hpp>

using ntf::vec2;
using ntf::vec3;
using ntf::vec4;
using ntf::ivec2;
using ntf::cmplx;

using ntf::mat3;
using ntf::mat4;

using ntf::color3;
using ntf::color4;

using ntf::shogle::transform2d;
using ntf::shogle::transform3d;

using ntf::shogle::sprite;
using ntf::shogle::sprite_group;
using ntf::shogle::sprite_animator;
using ntf::shogle::sprite_group;
using ntf::shogle::spritesheet;

using ntf::shogle::font;

using ntf::shogle::shader_program;

using ntf::shogle::framebuffer;

using ntf::shogle::keycode;
using ntf::shogle::keystate;

using ntf::strmap;
using std::vector;

const constexpr uint UPS = 60;
const constexpr float DT = 1.f/UPS;
const constexpr ivec2 WIN_SIZE {1280, 720};
const constexpr ivec2 VIEWPORT = {880, 660};
// const constexpr ivec2 VIEWPORT {768, 896};
const constexpr auto fb_ratio = 6.f/7.f;

#pragma once

#include <shogle.hpp>

#include <sol/forward.hpp>

#include <list>
#include <stack>

namespace okuu {

using real = float;

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

using logger = ntf::logger;

using renderer = ntf::gl_context;
using window = ntf::glfw_window<okuu::renderer>;

using keycode = ntf::glfw_keycode;
using keystate = ntf::glfw_keystate;

template<typename T>
using ref = std::reference_wrapper<T>;

const constexpr uint UPS = 60;
const constexpr float DT = 1.f/UPS;
const constexpr float VIEWPORT_RATIO = 6.f/7.f;

const constexpr ivec2 WIN_SIZE {1280, 720};
const constexpr ivec2 VIEWPORT {600, 700};
// const constexpr ivec2 VIEWPORT = {880, 660};

constexpr std::string_view shogle_ver = "shogle ver. " SHOGLE_VERSION_STRING;
constexpr std::string_view stlib_key = "okuu";

class context;

} // namespace okuu

#pragma once

#include <shogle/math/alg.hpp>

#include <shogle/render/gl.hpp>
#include <shogle/render/glfw.hpp>
#include <shogle/render/imgui.hpp>

#include <shogle/engine.hpp>

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

using logger = ntf::log;
using renderer = ntf::gl_renderer;
using glfw = ntf::glfw;
using imgui = ntf::imgui;

using window_type = glfw::window<renderer>;
using imgui_type = imgui::imgui_lib<imgui::glfw_gl3_impl>;

const constexpr uint UPS = 60;
const constexpr float DT = 1.f/UPS;
const constexpr float VIEWPORT_RATIO = 6.f/7.f;

const constexpr ivec2 WIN_SIZE {1280, 720};
const constexpr ivec2 VIEWPORT {600, 700};

constexpr std::string_view stlib_key = "okuu";
// const constexpr ivec2 VIEWPORT = {880, 660};

namespace global {

enum class states {
  loading = 0,
  frontend,
  gameplay,
};

struct global_state {
  states current_state{states::loading};

  frames elapsed_ticks{0};
  frames elapsed_frames{0};
};

void tick();
void start_stage(std::string path);
void go_back();

global_state& state();

} // namespace global

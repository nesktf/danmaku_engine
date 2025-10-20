#pragma once

#include <shogle/shogle.hpp>

namespace okuu::render {

using namespace ntf::numdefs;
using shogle::mat4;
using shogle::uvec2;
using shogle::vec2;
using shogle::vec3;

template<typename T>
using expect = ntf::expected<T, std::string>;

enum class viewport_res {
  x480p = 0, // by 560 (6:7), 640 (4:3), 360 (16:9)
  x600p,
  x720p,
  x900p,
  x1024p,
  x1080p,
};

struct singleton_handle {
  singleton_handle() noexcept = default;
  ~singleton_handle() noexcept;
};

[[nodiscard]] singleton_handle init();

shogle::window& window();

shogle::context_view shogle_ctx();

void render_back(float t);

expect<shogle::pipeline> create_pipeline();

} // namespace okuu::render

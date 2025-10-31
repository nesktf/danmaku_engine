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

void render_back(float t);

expect<shogle::texture2d> create_texture(u32 width, u32 height, const void* data);

expect<std::pair<shogle::texture2d, shogle::framebuffer>> create_framebuffer(u32 width,
                                                                             u32 height);

enum class pipeline_attrib {
  sprite_generic = 0,
};

expect<shogle::pipeline> create_pipeline(std::string_view frag_src, pipeline_attrib attrib);

expect<shogle::shader_storage_buffer> create_ssbo(size_t size, const void* data = nullptr);

// struct sprite_render_data {
//   shogle::framebuffer_view target;
//   shogle::pipeline_view pipeline;
//   shogle::texture2d_view texture;
//   shogle::shader_storage_buffer buffer;
//   ntf::inplace_function<void(shogle::shader_storage_buffer_view) const> on_render_buffer_write;
//   mat4 proj;
//   mat4 view;
// };

} // namespace okuu::render

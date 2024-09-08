#pragma once

#include "global.hpp"

#include <shogle/render/gl/shader.hpp>
#include <shogle/render/gl/font.hpp>
#include <shogle/render/gl/texture.hpp>

#include <shogle/assets/pool.hpp>
#include <shogle/assets/font.hpp>
#include <shogle/assets/atlas.hpp>

namespace res {

using pool_handle = ntf::resource_handle;

using texture_type = renderer::texture2d;
using texture_data = ntf::texture_data<texture_type>;

using shader_type = renderer::shader_program;

using font_type = renderer::font;
using font_data = ntf::font_data<font_type>;

using atlas_type = ntf::texture_atlas<texture_type>;
using atlas_data = atlas_type::data_type;

template<typename Resource, typename Getter>
class handle_wrapper {
public:
  handle_wrapper() = default;

  handle_wrapper(pool_handle id) :
    _id(id) {}

public:
  pool_handle id() const { return _id; }

  const Resource& get() const { return Getter{}(_id); }
  const Resource* operator->() const { return &get(); }
  const Resource& operator*() const { return get(); }
  operator const Resource&() const { return get(); }

private:
  pool_handle _id{0};
};

struct shader_getter { const shader_type& operator()(pool_handle id); };
struct font_getter { const font_type& operator()(pool_handle id); };
struct atlas_getter { const atlas_type& operator()(pool_handle id); };
struct texture_getter { const texture_type& operator()(pool_handle id); };

using shader = handle_wrapper<shader_type, shader_getter>;
using font = handle_wrapper<font_type, font_getter>;
using atlas = handle_wrapper<atlas_type, atlas_getter>;
using texture = handle_wrapper<texture_type, texture_getter>;

struct sprite {
  atlas atlas_handle{0};
  atlas_type::texture_handle atlas_index{0};
  std::optional<atlas_type::sequence_handle> sequence{};
};

void init(std::function<void()> callback);
void destroy();

void request_shader(std::string name, std::string vert_path, std::string frag_path);
void request_texture(std::string name, std::string path);
void request_font(std::string name, std::string path);
void request_atlas(std::string name, std::string path);

void start_loading(std::function<void()> callback);
void do_requests();

std::optional<shader> shader_from_name(std::string_view name);
std::optional<font> font_from_name(std::string_view name);
std::optional<atlas> atlas_from_name(std::string_view name);
std::optional<texture> texture_from_name(std::string_view name);

void free(shader shader);
void free(font font);
void free(atlas atlas);
void free(texture texture);

} // namespace res

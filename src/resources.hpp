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

using sprite_animator = ntf::texture_animator<texture_type, atlas>;

struct sprite {
  atlas handle{0};
  atlas_type::texture_handle index{0};
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

std::optional<shader> get_shader(std::string_view name);
std::optional<font> get_font(std::string_view name);
std::optional<atlas> get_atlas(std::string_view name);
std::optional<atlas_type::sequence_handle> get_atlas_sequence(std::string_view atlas, std::string_view seq);
std::optional<atlas_type::sequence_handle> get_atlas_sequence(atlas handle, std::string_view seq);
std::optional<texture> get_texture(std::string_view name);

sprite sprite_from_index(atlas handle, atlas_type::texture_handle index);
sprite sprite_from_index(std::string_view atlas, atlas_type::texture_handle index);

sprite sprite_from_group(atlas handle, atlas_type::group_handle group, atlas_type::texture_handle index);
sprite sprite_from_group(atlas handle, std::string_view group, atlas_type::texture_handle index);
sprite sprite_from_group(std::string_view atlas, std::string_view group, atlas_type::texture_handle index);

sprite sprite_from_sequence(atlas handle, atlas_type::sequence_handle seq);
sprite sprite_from_sequence(atlas handle, std::string_view seq);
sprite sprite_from_sequence(std::string_view atlas, std::string_view seq);

void free(shader shader);
void free(font font);
void free(atlas atlas);
void free(texture texture);

} // namespace res

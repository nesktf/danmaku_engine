#pragma once

#include "global.hpp"

#include <shogle/render/gl/shader.hpp>
#include <shogle/render/gl/font.hpp>
#include <shogle/render/gl/texture.hpp>

#include <shogle/assets/pool.hpp>
#include <shogle/assets/font.hpp>
#include <shogle/assets/atlas.hpp>

#include <optional>

namespace res {

using pool_id = ntf::resource_id;

using texture_type = renderer::texture2d;
using texture_data = ntf::texture_data<texture_type>;

using shader_type = renderer::shader_program;

using font_type = renderer::font;
using font_data = ntf::font_data<font_type>;

using atlas_type = ntf::texture_atlas<texture_type>;
using atlas_data = atlas_type::data_type;

template<typename Resource, typename Getter>
class handle {
public:
  handle() = default;

  handle(pool_id id) :
    _id(id) {}

public:
  pool_id id() const { return _id; }

  const Resource& get() const { return Getter{}(_id); }
  const Resource* operator->() const { return &get(); }
  const Resource& operator*() const { return get(); }
  operator const Resource&() const { return get(); }

private:
  pool_id _id{0};
};

struct shader_getter { const shader_type& operator()(pool_id id); };
struct font_getter { const font_type& operator()(pool_id id); };
struct atlas_getter { const atlas_type& operator()(pool_id id); };
struct texture_getter { const texture_type& operator()(pool_id id); };

using shader = handle<shader_type, shader_getter>;
using font = handle<font_type, font_getter>;
using atlas = handle<atlas_type, atlas_getter>;
using texture = handle<texture_type, texture_getter>;


void init();
void destroy();

void request_shader(std::string name, std::string vert_path, std::string frag_path);
void request_texture(std::string name, std::string path);
void request_font(std::string name, std::string path);
void request_atlas(std::string name, std::string path);

void start_loading();
void do_requests();
void set_callback(std::function<void()> cb);

std::optional<shader> shader_from_name(std::string_view name);
std::optional<font> font_from_name(std::string_view name);
std::optional<atlas> atlas_from_name(std::string_view name);
std::optional<texture> texture_from_name(std::string_view name);

void free(shader shader);
void free(font font);
void free(atlas atlas);
void free(texture texture);

} // namespace res

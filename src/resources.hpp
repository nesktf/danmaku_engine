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
struct shader_data {
  struct loader {
    shader_type operator()(shader_data data);
    shader_type operator()(std::string vert, std::string frag);
  };

  shader_data(std::string vert_, std::string frag_) :
    vert(std::move(vert_)), frag(std::move(frag_)) {}

  std::string vert, frag;
};

using font_type = renderer::font;
using font_data = ntf::font_data<font_type>;

using atlas_type = ntf::texture_atlas<texture_type>;
using atlas_data = atlas_type::data_type;

class manager;

template<typename T>
class res_view {
public:
  res_view() = default;

  res_view(const manager* manager, pool_handle id) :
    _manager(manager), _id(id) {}

public:
  pool_handle pool_id() const { return _id; }

  const T& get() const;
  const T* operator->() const { return &get(); }
  const T& operator*() const { return get(); }
  explicit operator const T&() const { return get(); }

private:
  const manager* _manager;
  pool_handle _id;
};

using shader = res_view<shader_type>;
using font = res_view<font_type>;
using texture = res_view<texture_type>;

using atlas = res_view<atlas_type>;
using atlas_entry = atlas_type::texture_handle;
using atlas_sequence = atlas_type::sequence_handle;
using atlas_group = atlas_type::group_handle;

using sprite = std::pair<atlas, atlas_entry>;
using sprite_animator = ntf::texture_animator<atlas_type::texture_type, atlas>;
using sequence_pair = std::pair<atlas, atlas_sequence>;
using group_pair = std::pair<atlas, atlas_group>;

template<typename... Ts>
using request_vec = std::vector<std::tuple<Ts...>>;

class manager {
public:
  manager() = default;

public:
  void load_defaults();
  void init_requests(ntf::async_data_loader& loader, std::function<void()> callback);

  void request_shader(std::string name, std::string vert_path, std::string frag_path);
  void request_font(std::string name, std::string path);
  void request_atlas(std::string name, std::string path);
  void request_texture(std::string name, std::string path);

  void emplace_shader(std::string name, shader_type shader);
  void emplace_font(std::string name, font_type font);
  void emplace_atlas(std::string name, atlas_type atlas);
  void emplace_texture(std::string name, texture_type texture);

  void free(font font);
  void free(shader shader);
  void free(texture texture);
  void free(atlas atlas);

  font font_at(std::string_view name) const;
  shader shader_at(std::string_view name) const;
  texture texture_at(std::string_view name) const;
  atlas atlas_at(std::string_view name) const;
  sequence_pair sequence_at(std::string_view atlas, std::string_view seq) const;
  group_pair group_at(std::string_view atlas, std::string_view seq) const;

  std::optional<font> font_opt(std::string_view name) const;
  std::optional<shader> shader_opt(std::string_view name) const;
  std::optional<texture> texture_opt(std::string_view name) const;
  std::optional<atlas> atlas_opt(std::string_view name) const;
  std::optional<sequence_pair> sequence_opt(std::string_view atlas, std::string_view seq) const;
  std::optional<group_pair> group_opt(std::string_view atlas, std::string_view group) const;

  void clear();

public:
  std::size_t shader_count() const { return _shaders.size(); }
  std::size_t texture_count() const { return _textures.size(); }
  std::size_t font_count() const { return _fonts.size(); }
  std::size_t atlas_count() const { return _atlas.size(); }

  template<typename T>
  T& at(pool_handle id);

  template<>
  font_type& at<font_type>(pool_handle id) { return _fonts.at(id); }

  template<>
  shader_type& at<shader_type>(pool_handle id) { return _shaders.at(id); }

  template<>
  atlas_type& at<atlas_type>(pool_handle id) { return _atlas.at(id); }

  template<>
  texture_type& at<texture_type>(pool_handle id) { return _textures.at(id); }

private:
  void _clear_req();

private:
  request_vec<std::string, std::string, std::string> _shader_req;
  request_vec<std::string, std::string> _texture_req;
  request_vec<std::string, std::string> _font_req;
  request_vec<std::string, std::string> _atlas_req;

  ntf::resource_pool<shader_type, shader_data> _shaders;
  ntf::resource_pool<texture_type, texture_data> _textures;
  ntf::resource_pool<font_type, font_data> _fonts;
  ntf::resource_pool<atlas_type, atlas_data> _atlas;

  size_t _async_count{0};
};

texture_type default_texture();

template<typename T>
const T& res_view<T>::get() const {
  assert(_manager != nullptr && "Invalid manager!");
  return _manager->at<T>(_id);
}

} // namespace res

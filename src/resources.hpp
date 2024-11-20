#pragma once

#include "okuu.hpp"

namespace okuu {

using uniform = okuu::renderer::uniform;
using texture = okuu::renderer::texture2d;
using shader = okuu::renderer::program;
using font = okuu::renderer::font;
using framebuffer = okuu::renderer::framebuffer;

using atlas = ntf::texture_atlas<okuu::texture>;
using atlas_texture = ntf::atlas_texture;
using atlas_sequence = ntf::atlas_sequence;
using atlas_group = ntf::atlas_group;

using resource_handle = ntf::resource_handle;
using texture_data = ntf::texture_data<okuu::texture>;
using font_data = ntf::font_data<okuu::font>;
using atlas_data = okuu::atlas::data_type;

struct shader_data {
  struct loader {
    okuu::shader operator()(shader_data data);
    okuu::shader operator()(std::string vert, std::string frag);
  };

  shader_data(std::string vert_, std::string frag_) :
    vert(std::move(vert_)), frag(std::move(frag_)) {}

  std::string vert, frag;
};

class resource_manager;

template<typename T>
class resource {
public:
  resource() = default;

private:
  resource(okuu::resource_manager& manager, okuu::resource_handle id) :
    _manager(&manager), _id(id) {}

public:
  T& get();
  T* operator->() { return &get(); }
  T& operator*() { return get(); }

  const T& get() const;
  const T* operator->() const { return &get(); }
  const T& operator*() const { return get(); }

  bool valid() const { return _id != ntf::resource_tombstone && _manager != nullptr; }
  explicit operator bool() const { return valid(); }

private:
  okuu::resource_manager* _manager{nullptr};
  okuu::resource_handle _id{ntf::resource_tombstone};

private:
  friend class resource_manager;
};

using sprite = std::pair<resource<okuu::atlas>, okuu::atlas_texture>;
using sprite_sequence = std::pair<okuu::resource<okuu::atlas>, okuu::atlas_sequence>;
using sprite_group = std::pair<okuu::resource<okuu::atlas>, okuu::atlas_group>;

using sprite_animator = ntf::texture_animator<okuu::texture, okuu::resource<okuu::atlas>>;

template<typename... Ts>
using request_vec = std::vector<std::tuple<Ts...>>;

class resource_manager {
private:
  resource_manager() = default;

public:
  void load_defaults();
  void init_requests(ntf::async_data_loader& loader, std::function<void()> callback);

  void request_shader(std::string name, std::string vert_path, std::string frag_path);
  void request_font(std::string name, std::string path);
  void request_atlas(std::string name, std::string path);
  void request_texture(std::string name, std::string path);

  void emplace_shader(std::string name, okuu::shader shader);
  void emplace_font(std::string name, okuu::font font);
  void emplace_atlas(std::string name, okuu::atlas atlas);
  void emplace_texture(std::string name, okuu::texture texture);

  void free(okuu::resource<okuu::font> font);
  void free(okuu::resource<okuu::shader> shader);
  void free(okuu::resource<okuu::texture> texture);
  void free(okuu::resource<okuu::atlas> atlas);

  okuu::resource<okuu::font> font_at(std::string_view name) const;
  okuu::resource<okuu::shader> shader_at(std::string_view name) const;
  okuu::resource<okuu::texture> texture_at(std::string_view name) const;
  okuu::resource<okuu::atlas> atlas_at(std::string_view name) const;
  okuu::sprite_sequence sequence_at(std::string_view atlas, std::string_view seq) const;
  okuu::sprite_group group_at(std::string_view atlas, std::string_view grp) const;

  void clear();

public:
  std::size_t shader_count() const { return _shaders.size(); }
  std::size_t texture_count() const { return _textures.size(); }
  std::size_t font_count() const { return _fonts.size(); }
  std::size_t atlas_count() const { return _atlas.size(); }

  template<typename T>
  T& at(okuu::resource_handle) { NTF_ASSERT(false, "Invalid resource type"); }

  template<>
  okuu::font& at<okuu::font>(okuu::resource_handle id) { return _fonts.at(id); }

  template<>
  okuu::shader& at<okuu::shader>(okuu::resource_handle id) { return _shaders.at(id); }

  template<>
  okuu::atlas& at<okuu::atlas>(okuu::resource_handle id) { return _atlas.at(id); }

  template<>
  okuu::texture& at<okuu::texture>(okuu::resource_handle id) { return _textures.at(id); }

private:
  void _clear_req();

private:
  okuu::request_vec<std::string, std::string, std::string> _shader_req;
  okuu::request_vec<std::string, std::string> _texture_req;
  okuu::request_vec<std::string, std::string> _font_req;
  okuu::request_vec<std::string, std::string> _atlas_req;

  ntf::resource_pool<okuu::shader, okuu::shader_data> _shaders;
  ntf::resource_pool<okuu::texture, okuu::texture_data> _textures;
  ntf::resource_pool<okuu::font, okuu::font_data> _fonts;
  ntf::resource_pool<okuu::atlas, okuu::atlas_data> _atlas;

  std::size_t _async_count{0};

private:
  friend class okuu::context;
};

okuu::texture default_texture();

template<typename T>
T& resource<T>::get() {
  NTF_ASSERT(valid(), "Invalid resource");
  return _manager->at<T>(_id);
}

template<typename T>
const T& resource<T>::get() const {
  NTF_ASSERT(valid(), "Invalid resource");
  return _manager->at<T>(_id);
}

} // namespace okuu

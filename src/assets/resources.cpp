#include "resources.hpp"

namespace {

struct shader_data {
  struct loader {
    res::shader_type operator()(shader_data data) {
      res::shader_type::loader loader;
      return loader(ntf::file_contents(data.vert), ntf::file_contents(data.frag));
    }
    res::shader_type operator()(std::string vert, std::string frag) {
      return (*this)(shader_data{std::move(vert), std::move(frag)});
    }
  };
  shader_data(std::string vert_, std::string frag_) :
    vert(std::move(vert_)), frag(std::move(frag_)) {};
  std::string vert, frag;
};

struct {
  std::vector<std::tuple<std::string, std::string, std::string>> shader_req;
  std::vector<std::tuple<std::string, std::string>> texture_req;
  std::vector<std::tuple<std::string, std::string>> font_req;
  std::vector<std::tuple<std::string, std::string>> atlas_req;

  ntf::resource_pool<res::shader_type, shader_data> shaders;
  ntf::resource_pool<res::texture_type, res::texture_data> textures;
  ntf::resource_pool<res::font_type, res::font_data> fonts;
  ntf::resource_pool<res::atlas_type, res::atlas_data> atlas;

  ntf::async_data_loader loader;
  uint async_count {0};
} _res;

res::texture_type default_texture() {
  uint8_t pixels[] = {0x0, 0x0, 0x0, 0xFE, 0x0, 0xFE, 0x0, 0x0, 0xFE, 0x0, 0xFE, 0x00};
  res::texture_type tex{&pixels[0], ivec2{2, 2}, ntf::tex_format::rgb};
  tex.set_filter(ntf::tex_filter::nearest);
  tex.set_wrap(ntf::tex_wrap::repeat);
  return tex;
}

} // namespace


namespace res {

void init(std::function<void()> callback) {
  // shaders
  request_shader("sprite", "res/shader/sprite.vs.glsl", "res/shader/sprite.fs.glsl");
  request_shader("font", "res/shader/font.vs.glsl", "res/shader/font.fs.glsl");
  request_shader("framebuffer", "res/shader/framebuffer.vs.glsl", "res/shader/framebuffer.fs.glsl");
  request_shader("frontend", "res/shader/frontend.vs.glsl", "res/shader/frontend.fs.glsl");

  // sprites
  _res.atlas.emplace("default", atlas_type{default_texture()});
  request_atlas("enemies", "res/spritesheet/enemies.json");
  request_atlas("effects", "res/spritesheet/effects.json");
  request_atlas("chara_reimu", "res/spritesheet/chara_reimu.json");
  request_atlas("chara_marisa", "res/spritesheet/chara_marisa.json");
  request_atlas("chara_cirno", "res/spritesheet/chara_cirno.json");

  // fonts
  request_font("arial", "res/fonts/arial.ttf");

  // textures
  _res.textures.emplace("default", default_texture());

  start_loading(callback);
}

void destroy() {
  _res.shaders.clear();
  _res.fonts.clear();
  _res.atlas.clear();
  _res.textures.clear();
}

void request_shader(std::string name, std::string vert_path, std::string frag_path) {
  _res.shader_req.emplace_back(
    std::make_tuple(std::move(name), std::move(vert_path), std::move(frag_path))
  );
}

void request_font(std::string name, std::string path) {
  _res.font_req.emplace_back(
    std::make_tuple(std::move(name), std::move(path))
  );
}

void request_atlas(std::string name, std::string path) {
  _res.atlas_req.emplace_back(
    std::make_tuple(std::move(name), std::move(path))
  );
}

void do_requests() {
  _res.loader.do_requests();
}

bool has_requests() {
  return _res.atlas_req.size() > 0 || _res.font_req.size() > 0 ||
         _res.texture_req.size() > 0 || _res.shader_req.size() > 0;
}

void start_loading(std::function<void()> callback) {
  size_t res_total = _res.shader_req.size() + _res.font_req.size() +
                     _res.atlas_req.size() + _res.texture_req.size();

  auto on_load = [=](pool_handle handle, std::string name) {
    ntf::log::debug("[res::start_loading] Resource loaded, {} (id: {})", name, handle);
    if (++_res.async_count < res_total) {
      return;
    }
    ntf::log::debug("[res::start_loading] Loading complete, {} items", res_total);
    callback();
  };

  _res.async_count = 0;
  ntf::tex_filter def_filter = ntf::tex_filter::nearest;
  ntf::tex_wrap def_wrap = ntf::tex_wrap::repeat;

  for (auto& [name, vert_path, frag_path] : _res.shader_req) {
    _res.shaders.enqueue(std::move(name), _res.loader, on_load,
                         std::move(vert_path), std::move(frag_path));
  }

  for (auto& [name, path] : _res.font_req) {
    _res.fonts.enqueue(std::move(name), _res.loader, on_load, 
                       std::move(path));
  }

  for (auto& [name, path] : _res.atlas_req) {
    _res.atlas.enqueue(std::move(name), _res.loader, on_load,
                       std::move(path), def_filter, def_wrap);
  }

  for (auto& [name, path] : _res.texture_req) {
    _res.textures.enqueue(std::move(name), _res.loader, on_load,
                          std::move(path), def_filter, def_wrap);
  }

  _res.shader_req.clear();
  _res.font_req.clear();
  _res.atlas_req.clear();
  ntf::log::debug("[res::start_loading] Enqueued {} resources", res_total);
}

const shader_type& shader_getter::operator()(pool_handle id) { return _res.shaders[id]; }
const font_type& font_getter::operator()(pool_handle id) { return _res.fonts[id]; }
const atlas_type& atlas_getter::operator()(pool_handle id) { return _res.atlas[id]; }
const texture_type& texture_getter::operator()(pool_handle id) { return _res.textures[id]; }

std::optional<shader> get_shader(std::string_view name) {
  auto shader = _res.shaders.find(name);
  if (shader) {
    return std::make_optional(static_cast<pool_handle>(shader.value()));
  }
  ntf::log::warning("[res::shader_from_name] Shader not found: \"{}\"", name);
  return {};
}

std::optional<font> get_font(std::string_view name) {
  auto font = _res.fonts.find(name);
  if (font) {
    return std::make_optional(static_cast<pool_handle>(font.value()));
  }
  ntf::log::warning("[res::font_from_name] Font not found: \"{}\"", name);
  return {};
}

std::optional<atlas> get_atlas(std::string_view name) {
  auto atlas = _res.atlas.find(name);
  if (atlas) {
    return std::make_optional(static_cast<pool_handle>(atlas.value()));
  }
  ntf::log::warning("[res::atlas_from_name] Atlas not found: \"{}\"", name);
  return {};
}

std::optional<atlas_type::sequence_handle> get_atlas_sequence(std::string_view atlas, std::string_view seq) {
  auto atlas_handle = get_atlas(atlas);
  if (!atlas_handle) {
    ntf::log::warning("[res::get_atlas_sequence] Atlas not found: \"{}\"", atlas);
    return {};
  }
  auto seq_handle = atlas_handle.value()->find_sequence(seq);
  if (!seq_handle) {
    ntf::log::warning("[res::get_atlas_sequence] Sequence \"{}\" not found in atlas \"{}\"", seq, atlas);
    return {};
  }
  return seq_handle.value();
}

std::optional<atlas_type::sequence_handle> get_atlas_sequence(atlas atlas_handle, std::string_view seq) {
  auto seq_handle = atlas_handle->find_sequence(seq);
  if (!seq_handle) {
    ntf::log::warning("[res::get_atlas_sequence] Sequence \"{}\" not found in atlas \"{}\"", seq, atlas_handle.id());
    return {};
  }
  return seq_handle.value();
}

std::optional<texture> get_texture(std::string_view name) {
  auto texture = _res.textures.find(name);
  if (texture) {
    return std::make_optional(static_cast<pool_handle>(texture.value()));
  }
  ntf::log::warning("[res::texture_from_name] Texture not found: \"{}\"", name);
  return {};
}

void free(shader shader) {
  ntf::log::debug("[res::free] Shader slot {} issued for reuse", shader.id());
  _res.shaders.unload(shader.id());
}

void free(font font) {
  ntf::log::debug("[res::free] Font slot {} issued for reuse", font.id());
  _res.fonts.unload(font.id());
}

void free(atlas atlas) {
  ntf::log::debug("[res::free] Atlas slot {} issued for reuse", atlas.id());
  _res.atlas.unload(atlas.id());
}

void free(texture texture) {
  ntf::log::debug("[res::free] Texture slot {} issued for reuse", texture.id());
  _res.textures.unload(texture.id());
}
//
// static inline sprite default_sprite() {
//   return sprite{.handle={0}, .index=0};
// }
//
// sprite sprite_from_index(atlas handle, atlas_type::texture_handle index) {
//   assert(handle->size() > index && "Invalid atlas index");
//   return sprite {.handle = handle, .index = index};
// }
//
// sprite sprite_from_index(std::string_view atlas, atlas_type::texture_handle index) {
//   auto atlas_handle = get_atlas(atlas);
//   if (!atlas_handle) {
//     ntf::log::warning("[res::sprite_from_index] Atlas not found \"{}\"", atlas);
//     return default_sprite();
//   }
//   if (atlas_handle.value()->size() <= index) {
//     ntf::log::warning("[res::sprite_from_index] Index \"{}\" out of bounds for atlas \"{}\"", atlas, index);
//     return default_sprite();
//   }
//   return sprite_from_index(atlas_handle.value(), index);
// }
//
// sprite sprite_from_group(atlas handle, atlas_type::group_handle group, atlas_type::texture_handle index) {
//   assert(handle->size() > index && "Invalid atlas index");
//   assert(handle->group_count() > group && "Invalid atlas group");
//   return sprite{.handle = handle, .index = handle->group_at(group)[index]};
// }
//
// sprite sprite_from_group(atlas handle, std::string_view group, atlas_type::texture_handle index) {
//   if (handle->size() <= index) {
//     ntf::log::warning("[res::sprite_from_group] Index \"{}\" out of bounds for atlas \"{}\"", handle.id(), index);
//     return default_sprite();
//   }
//   auto group_handle = handle->find_group(group);
//   if (!group_handle) {
//     ntf::log::warning("[res::sprite_from_group] Group \"{}\" not found in atlas \"{}\"", group, handle.id());
//     return default_sprite();
//   }
//   return sprite_from_group(handle, group_handle.value(), index);
// }
//
// sprite sprite_from_group(std::string_view atlas, std::string_view group, atlas_type::texture_handle index) {
//   auto atlas_handle = get_atlas(atlas);
//   if (!atlas_handle) {
//     ntf::log::warning("[res::sprite_from_group] Atlas not found \"{}\"", atlas);
//     return default_sprite();
//   }
//   if (atlas_handle.value()->size() <= index) {
//     ntf::log::warning("[res::sprite_from_group] Index \"{}\" out of bounds for atlas \"{}\"", atlas, index);
//     return default_sprite();
//   }
//   auto group_handle = atlas_handle.value()->find_group(group);
//   if (!group_handle) {
//     ntf::log::warning("[res::sprite_from_group] Group \"{}\" not found in atlas \"{}\"", group, atlas);
//     return default_sprite();
//   }
//   return sprite_from_group(atlas_handle.value(), group_handle.value(), index);
// }
//
// sprite sprite_from_sequence(atlas handle, atlas_type::sequence_handle seq) {
//   assert(handle->size() > seq && "Invalid atlas sequence");
//   return sprite{.handle = handle, .sequence = seq};
// }
//
// sprite sprite_from_sequence(atlas handle, std::string_view seq) {
//   auto seq_handle = handle->find_sequence(seq);
//   if (!seq_handle) {
//     ntf::log::warning("[res::sprite_from_sequence] Sequence \"{}\" not found in atlas \"{}\"", seq, handle.id());
//     return default_sprite();
//   }
//   return sprite_from_sequence(handle, seq_handle.value());
// }
//
// sprite sprite_from_sequence(std::string_view atlas, std::string_view seq) {
//   auto atlas_handle = get_atlas(atlas);
//   if (!atlas_handle) {
//     ntf::log::warning("[res::sprite_from_sequence] Atlas not found \"{}\"", atlas);
//     return default_sprite();
//   }
//   auto seq_handle = atlas_handle.value()->find_sequence(seq);
//   if (!seq_handle) {
//     ntf::log::warning("[res::sprite_from_sequence] Sequence \"{}\" not found in atlas \"{}\"", seq, atlas);
//     return default_sprite();
//   }
//   return sprite_from_sequence(atlas_handle.value(), seq_handle.value());
// }
//

} // namespace res

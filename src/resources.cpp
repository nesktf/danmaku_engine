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

std::optional<shader> shader_from_name(std::string_view name) {
  auto shader = _res.shaders.find(name);
  if (shader) {
    return std::make_optional(static_cast<pool_handle>(shader.value()));
  }
  ntf::log::warning("[res::shader_from_name] Shader not found: \"{}\"", name);
  return {};
}

std::optional<font> font_from_name(std::string_view name) {
  auto font = _res.fonts.find(name);
  if (font) {
    return std::make_optional(static_cast<pool_handle>(font.value()));
  }
  ntf::log::warning("[res::font_from_name] Font not found: \"{}\"", name);
  return {};
}

std::optional<atlas> atlas_from_name(std::string_view name) {
  auto atlas = _res.atlas.find(name);
  if (atlas) {
    return std::make_optional(static_cast<pool_handle>(atlas.value()));
  }
  ntf::log::warning("[res::atlas_from_name] Atlas not found: \"{}\"", name);
  return {};
}

std::optional<texture> texture_from_name(std::string_view name) {
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

} // namespace res

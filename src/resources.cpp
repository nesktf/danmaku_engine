#include "resources.hpp"

namespace {

struct {
  std::vector<std::tuple<std::string, std::string, std::string>> shader_req;
  std::vector<std::tuple<std::string, std::string>> font_req;
  std::vector<std::tuple<std::string, std::string>> atlas_req;

  ntf::resource_pool<res::shader_type> shaders;
  ntf::resource_pool<res::texture_type, res::texture_data> textures;
  ntf::resource_pool<res::font_type, res::font_data> fonts;
  ntf::resource_pool<res::atlas_type, res::atlas_data> atlas;

  ntf::async_data_loader loader;
  uint async_count {0};
  uint async_total {0};

  std::function<void()> on_load;
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

void init() {
  // shaders
  request_shader("sprite", "res/shader/sprite.vs.glsl", "res/shader/sprite.fs.glsl");
  request_shader("font", "res/shader/font.vs.glsl", "res/shader/font.fs.glsl");
  request_shader("framebuffer", "res/shader/framebuffer.vs.glsl", "res/shader/framebuffer.fs.glsl");
  request_shader("frontend", "res/shader/frontend.vs.glsl", "res/shader/frontend.fs.glsl");

  // sprites
  _res.atlas.emplace("default", atlas_type{default_texture()});
  request_atlas("enemies", "res/spritesheet/enemies.json");
  request_atlas("effects", "res/spritesheet/effects.json");
  request_atlas("chara", "res/spritesheet/chara.json");

  // fonts
  request_font("arial", "res/fonts/arial.ttf");

  // textures
  _res.textures.emplace("default", default_texture());

  start_loading();
}

void destroy() {
  _res.shaders.clear();
  _res.fonts.clear();
  _res.atlas.clear();
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

void set_callback(std::function<void()> cb) {
  _res.on_load = std::move(cb);
}

void do_requests() {
  _res.loader.do_requests();
}

void start_loading() {
  auto callback = [](auto) {
    if (++_res.async_count < _res.async_total) {
      return;
    }

    _res.shader_req.clear();
    _res.font_req.clear();
    _res.atlas_req.clear();

    ntf::log::debug("[res::start_loading] Loading complete: {} items", _res.async_total);
    _res.on_load();
    _res.on_load = {};
  };

  _res.async_count = 0;
  ntf::tex_filter def_filter = ntf::tex_filter::nearest;
  ntf::tex_wrap def_wrap = ntf::tex_wrap::repeat;

  // TODO: add shader_data to load this with the loader
  for (auto& [name, vert_path, frag_path] : _res.shader_req) {
    _res.shaders.emplace(name, vert_path, frag_path);
  }
  _res.async_count += _res.shader_req.size();

  for (auto& [name, path] : _res.font_req) {
    _res.fonts.enqueue(std::move(name), _res.loader, callback, 
                       std::move(path));
  }

  for (auto& [name, path] : _res.atlas_req) {
    _res.atlas.enqueue(std::move(name), _res.loader, callback,
                       std::move(path), def_filter, def_wrap);
  }
  _res.async_total = _res.shader_req.size() + _res.font_req.size() +
                     _res.atlas_req.size();
}

const shader_type& shader_getter::operator()(pool_id id) { return _res.shaders[id]; }
const font_type& font_getter::operator()(pool_id id) { return _res.fonts[id]; }
const atlas_type& atlas_getter::operator()(pool_id id) { return _res.atlas[id]; }
const texture_type& texture_getter::operator()(pool_id id) { return _res.textures[id]; }

std::optional<shader> shader_from_name(std::string_view name) {
  if (_res.shaders.has(name)) {
    return std::make_optional(_res.shaders.id(name));
  }
  ntf::log::warning("[res::shader_from_name] Shader not found: \"{}\"", name);
  return {};
}

std::optional<font> font_from_name(std::string_view name) {
  if (_res.fonts.has(name)) {
    return std::make_optional(_res.fonts.id(name));
  }
  ntf::log::warning("[res::font_from_name] Font not found: \"{}\"", name);
  return {};
}

std::optional<atlas> atlas_from_name(std::string_view name) {
  if (_res.atlas.has(name)) {
    return std::make_optional(_res.atlas.id(name));
  }
  ntf::log::warning("[res::atlas_from_name] Atlas not found: \"{}\"", name);
  return {};
}

std::optional<texture> texture_from_name(std::string_view name) {
  if (_res.textures.has(name)) {
    return std::make_optional(_res.textures.id(name));
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

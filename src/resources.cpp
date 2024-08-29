#include "resources.hpp"

namespace {

using shader_program = renderer::shader_program;

using font = renderer::font;
using font_data = ntf::font_data<font>;

using texture = renderer::texture2d;
using texture_data = ntf::texture_data<texture>;

using atlas = ntf::texture_atlas<texture>;
using atlas_data = atlas::data_type;

using shader_id = ntf::resource_pool<shader_program>::resource_id;
using font_id = ntf::resource_pool<font, font_data>::resource_id;
using atlas_id = ntf::resource_pool<atlas, atlas_data>::resource_id;


struct {
  std::vector<std::tuple<std::string, std::string, std::string>> shader_req;
  std::vector<std::tuple<std::string, std::string>> font_req;
  std::vector<std::tuple<std::string, std::string>> atlas_req;

  ntf::resource_pool<shader_program> shaders;
  ntf::resource_pool<font, font_data> fonts;
  ntf::resource_pool<atlas, atlas_data> atlas;

  ntf::async_data_loader loader;
  uint async_count {0};
  uint async_total {0};

  std::function<void()> on_load;
} _resources;

} // namespace


namespace res {

void init() {
  // shaders
  request_shader("sprite", "res/shader/sprite.vs.glsl", "res/Shader/sprite.fs.glsl");
  request_shader("font", "res/shader/font.vs.glsl", "res/shader/font.fs.glsl");
  request_shader("framebuffer", "res/shader/framebuffer.vs.glsl", "res/shader/framebuffer.fs.glsl");
  request_shader("frontend", "res/shader/frontend.vs.glsl", "res/shader/frontend.fs.glsl");

  // sprites
  uint8_t pixels[] = {0x0, 0x0, 0x0, 0xFE, 0x0, 0xFE, 0x0, 0x0, 0xFE, 0x0, 0xFE, 0x00};
  renderer::texture2d tex{&pixels[0], ivec2{2, 2}, ntf::tex_format::rgb};
  tex.set_filter(ntf::tex_filter::nearest);
  tex.set_wrap(ntf::tex_wrap::repeat);

  _resources.atlas.emplace("default", atlas{std::move(tex)}); // funny default texture
  request_atlas("enemies", "res/spritesheet/enemies.json");
  request_atlas("effects", "res/spritesheet/effects.json");
  request_atlas("chara", "res/spritesheet/chara.json");

  // fonts
  request_font("arial", "res/fonts/arial.ttf");

  start_loading();
}

void destroy() {
  _resources.shaders.clear();
  _resources.fonts.clear();
  _resources.atlas.clear();
}

void request_shader(std::string name, std::string vert_path, std::string frag_path) {
  _resources.shader_req.emplace_back(
    std::make_tuple(std::move(name), std::move(vert_path), std::move(frag_path))
  );
}

void request_font(std::string name, std::string path) {
  _resources.font_req.emplace_back(
    std::make_tuple(std::move(name), std::move(path))
  );
}

void request_atlas(std::string name, std::string path) {
  _resources.atlas_req.emplace_back(
    std::make_tuple(std::move(name), std::move(path))
  );
}

void set_callback(std::function<void()> cb) {
  _resources.on_load = std::move(cb);
}

void do_requests() {
  _resources.loader.do_requests();
}

void start_loading() {
  auto callback = [](auto) {
    if (++_resources.async_count != _resources.async_total) {
      return;
    }

    _resources.shader_req.clear();
    _resources.font_req.clear();
    _resources.atlas_req.clear();

    _resources.on_load();
    _resources.on_load = {};
  };

  _resources.async_count = 0;
  ntf::tex_filter def_filter = ntf::tex_filter::nearest;
  ntf::tex_wrap def_wrap = ntf::tex_wrap::repeat;

  // TODO: add shader_data to load this with the loader
  for (auto& shader : _resources.shader_req) {
    auto& name = std::get<0>(shader);
    auto& vert_path = std::get<1>(shader);
    auto& frag_path = std::get<2>(shader);

    _resources.shaders.emplace(name, vert_path, frag_path);
  }
  _resources.async_count += _resources.shader_req.size();

  for (auto& font : _resources.font_req) {
    auto& name = std::get<0>(font);
    auto& path = std::get<1>(font);

    _resources.fonts.enqueue(std::move(name), _resources.loader, callback, 
                             std::move(path));
  }

  for (auto& atlas : _resources.atlas_req) {
    auto& name = std::get<0>(atlas);
    auto& path = std::get<1>(atlas);

    _resources.atlas.enqueue(std::move(name), _resources.loader, callback,
                             std::move(path), def_filter, def_wrap);
  }
  _resources.async_total = _resources.shader_req.size() + _resources.font_req.size() +
                           _resources.atlas_req.size();
}

shader::shader(std::string_view name) :
  _shader(_resources.shaders.id(name)) {}

const renderer::shader_program& shader::get() const {
  assert(valid() && "Invalid shader");
  return _resources.shaders[_shader];
}

bool shader::valid() const {
  return _resources.shaders.has(_shader);
}


sprite_atlas::sprite_atlas(std::string_view name) : 
  _atlas(_resources.atlas.id(name)) {}

const ntf::texture_atlas<renderer::texture2d>& sprite_atlas::get() const {
  assert(valid() && "Invalid atlas");
  return _resources.atlas[_atlas];
}

bool sprite_atlas::valid() const {
  return _resources.atlas.has(_atlas);
}

sprite sprite_atlas::at(ntf::texture_atlas<renderer::texture2d>::texture tex) const {
  return sprite{*this, tex};
}


const ntf::texture_atlas<renderer::texture2d>::texture_meta& sprite::meta() const {
  assert(valid() && "Invalid sprite");
  return _atlas.get()[_tex];
}

const renderer::texture2d& sprite::tex() const {
  assert(valid() && "Invalid sprite");
  return _atlas.get().tex();
}

bool sprite::valid() const {
  return _atlas.valid() && _atlas.get().has(_tex);
}


font::font(std::string_view name) :
  _font(_resources.fonts.id(name)) {}

const renderer::font& font::get() const {
  assert(valid() && "Invalid font");
  return _resources.fonts[_font];
}

bool font::valid() const {
  return _resources.fonts.has(_font);
}

} // namespace res

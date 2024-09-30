#include "resources.hpp"

namespace res {

shader_type shader_data::loader::operator()(shader_data data) {
  shader_type::loader loader;
  return loader(ntf::file_contents(data.vert), ntf::file_contents(data.frag));
}

shader_type shader_data::loader::operator()(std::string vert, std::string frag) {
  return (*this)(shader_data{std::move(vert), std::move(frag)});
}

void manager::load_defaults() {
  _atlas.emplace("default", atlas_type{default_texture()});
  _textures.emplace("default", default_texture());
}

void manager::init_requests(ntf::async_data_loader& loader, std::function<void()> callback) {
  size_t res_total = _shader_req.size() + _font_req.size() +
                     _atlas_req.size() + _texture_req.size();

  auto on_load = [=](pool_handle, std::string) {
    if (++_async_count < res_total) {
      return;
    }
    callback();
  };

  _async_count = 0;
  ntf::tex_filter filter{ntf::tex_filter::nearest};
  ntf::tex_wrap wrap{ntf::tex_wrap::repeat};

  for (auto& [name, vert_path, frag_path] : _shader_req) {
    _shaders.enqueue(std::move(name), loader, on_load,
                     std::move(vert_path), std::move(frag_path));
  }

  for (auto& [name, path] : _font_req) {
    _fonts.enqueue(std::move(name), loader, on_load,
                   std::move(path));
  }

  for (auto& [name, path] : _atlas_req) {
    _atlas.enqueue(std::move(name), loader, on_load,
                   std::move(path), filter, wrap);
  }

  for (auto& [name, path] : _texture_req) {
    _textures.enqueue(std::move(name), loader, on_load,
                      std::move(path), filter, wrap);
  }

  _clear_req();
}

void manager::request_shader(std::string name, std::string vert_path, std::string frag_path) {
  _shader_req.emplace_back(
    std::make_tuple(std::move(name), std::move(vert_path), std::move(frag_path))
  );
}

void manager::request_font(std::string name, std::string path) {
  _font_req.emplace_back(
    std::make_tuple(std::move(name), std::move(path))
  );
}

void manager::request_atlas(std::string name, std::string path) {
  _atlas_req.emplace_back(
    std::make_tuple(std::move(name), std::move(path))
  );
}

void manager::request_texture(std::string name, std::string path) {
  _texture_req.emplace_back(
    std::make_tuple(std::move(name), std::move(path))
  );
}

void manager::emplace_shader(std::string name, shader_type shader) {
  _shaders.emplace(std::move(name), std::move(shader));
}

void manager::emplace_font(std::string name, font_type font) {
  _fonts.emplace(std::move(name), std::move(font));
}

void manager::emplace_atlas(std::string name, atlas_type atlas) {
  _atlas.emplace(std::move(name), std::move(atlas));
}

void manager::emplace_texture(std::string name, texture_type texture) {
  _textures.emplace(std::move(name), std::move(texture));
}

void manager::free(font font) {
  _fonts.unload(font.pool_id());
}

void manager::free(shader shader) {
  _shaders.unload(shader.pool_id());
}

void manager::free(atlas atlas) {
  _atlas.unload(atlas.pool_id());
}

void manager::free(texture texture) {
  _textures.unload(texture.pool_id());
}

font manager::font_at(std::string_view name) const {
  auto found = _fonts.find(name);
  assert(found && "Font not found!");
  return font{this, found.value()};
}

shader manager::shader_at(std::string_view name) const {
  auto found = _shaders.find(name);
  assert(found && "Shader not found!");
  return shader{this, found.value()};
}

texture manager::texture_at(std::string_view name) const {
  auto found = _textures.find(name);
  assert(found && "Texture not found!");
  return texture{this, found.value()};
}

atlas manager::atlas_at(std::string_view name) const {
  auto found = _atlas.find(name);
  assert(found && "Atlas not found!");
  return atlas{this, found.value()};
}

sequence_pair manager::sequence_at(std::string_view atlas_name, std::string_view seq) const {
  auto found_atlas = _atlas.find(atlas_name);
  assert(found_atlas && "Atlas not found!");

  const auto& atl = _atlas.at(found_atlas.value());
  auto found_seq = atl.find_sequence(seq);
  assert(found_seq && "Sequence not found!");

  return sequence_pair{atlas{this, found_atlas.value()}, found_seq.value()};
}

group_pair manager::group_at(std::string_view atlas_name, std::string_view group) const {
  auto found_atlas = _atlas.find(atlas_name);
  assert(found_atlas && "Atlas not found!");

  const auto& atl = _atlas.at(found_atlas.value());
  auto found_group = atl.find_group(group);
  assert(found_group && "Group not found!");

  return group_pair{atlas{this, found_atlas.value()}, found_group.value()};
}

std::optional<font> manager::font_opt(std::string_view name) const {
  auto found = _fonts.find(name);
  if (!found) {
    return std::nullopt;
  }
  return {font{this, found.value()}};
}

std::optional<shader> manager::shader_opt(std::string_view name) const {
  auto found = _shaders.find(name);
  if (!found) {
    return std::nullopt;
  }
  return {shader{this, found.value()}};
}

std::optional<texture> manager::texture_opt(std::string_view name) const {
  auto found = _textures.find(name);
  if (!found) {
    return std::nullopt;
  }
  return {texture{this, found.value()}};
}

std::optional<atlas> manager::atlas_opt(std::string_view name) const {
  auto found = _atlas.find(name);
  if (!found) {
    return std::nullopt;
  }
  return {atlas{this, found.value()}};
}

std::optional<sequence_pair> manager::sequence_opt(std::string_view atlas_name,
                                                   std::string_view seq) const {
  auto found_atlas = _atlas.find(atlas_name);
  if (!found_atlas) {
    return std::nullopt;
  }

  const auto& atl = _atlas.at(found_atlas.value());
  auto found_seq = atl.find_sequence(seq);
  if (!found_seq) {
    return std::nullopt;
  }

  return {sequence_pair{atlas{this, found_atlas.value()}, found_seq.value()}};
}

std::optional<group_pair> manager::group_opt(std::string_view atlas_name,
                                             std::string_view group) const {
  auto found_atlas = _atlas.find(atlas_name);
  if (!found_atlas) {
    return std::nullopt;
  }

  const auto& atl = _atlas.at(found_atlas.value());
  auto found_group = atl.find_group(group);
  if (!found_group) {
    return std::nullopt;
  }

  return group_pair{atlas{this, found_atlas.value()}, found_group.value()};
}

void manager::clear() {
  _shaders.clear();
  _fonts.clear();
  _atlas.clear();
  _textures.clear();
}

void manager::_clear_req() {
  _shader_req.clear();
  _texture_req.clear();
  _font_req.clear();
  _atlas_req.clear();
}

texture_type default_texture() {
  uint8_t pixels[] = {0x0, 0x0, 0x0, 0xFE, 0x0, 0xFE, 0x0, 0x0, 0xFE, 0x0, 0xFE, 0x00};
  res::texture_type tex{&pixels[0], ivec2{2, 2}, ntf::tex_format::rgb};
  tex.set_filter(ntf::tex_filter::nearest);
  tex.set_wrap(ntf::tex_wrap::repeat);
  return tex;
}


// void init(std::function<void()> callback) {
//   // shaders
//   request_shader("sprite", "res/shader/sprite.vs.glsl", "res/shader/sprite.fs.glsl");
//   request_shader("font", "res/shader/font.vs.glsl", "res/shader/font.fs.glsl");
//   request_shader("framebuffer", "res/shader/framebuffer.vs.glsl", "res/shader/framebuffer.fs.glsl");
//   request_shader("frontend", "res/shader/frontend.vs.glsl", "res/shader/frontend.fs.glsl");
//
//   // sprites
//   _res.atlas.emplace("default", atlas_type{default_texture()});
//   request_atlas("enemies", "res/spritesheet/enemies.json");
//   request_atlas("effects", "res/spritesheet/effects.json");
//   request_atlas("chara_reimu", "res/spritesheet/chara_reimu.json");
//   request_atlas("chara_marisa", "res/spritesheet/chara_marisa.json");
//   request_atlas("chara_cirno", "res/spritesheet/chara_cirno.json");
//
//   // fonts
//   request_font("arial", "res/fonts/arial.ttf");
//
//   // textures
//   _res.textures.emplace("default", default_texture());
//
//   start_loading(callback);
// }

} // namespace res

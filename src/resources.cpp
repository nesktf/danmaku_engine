#include "resources.hpp"

#include <shogle/res/util.hpp>
#include <shogle/core/log.hpp>

static struct {
  ntf::resource_pool<ntf::shader_program> shaders;
  ntf::resource_pool<ntf::font> fonts;
  ntf::resource_pool<ntf::texture_atlas, ntf::texture_atlas_data> sprites;
} resources;

static ntf::shader_program load_shader(std::string_view vert_path, std::string_view frag_path) {
  ntf::shader_program prog;

  auto vert_src = ntf::file_contents(vert_path.data());
  auto frag_src = ntf::file_contents(frag_path.data());

  try {
    prog = ntf::load_shader_program(vert_src, frag_src);
  } catch(...) {
    ntf::log::error("[resources::load_shader] Failed to link shader");
    throw;
  }

  return prog;
}

static void load_shaders() {
  auto& shaders = resources.shaders;

  shaders.emplace("sprite", 
                  load_shader("res/shader/sprite.vs.glsl", "res/shader/sprite.fs.glsl"));
  shaders.emplace("font", 
                  load_shader("res/shader/font.vs.glsl", "res/shader/font.fs.glsl"));
  shaders.emplace("framebuffer", 
                  load_shader("res/shader/framebuffer.vs.glsl", "res/shader/framebuffer.fs.glsl"));
  shaders.emplace("frontend", 
                  load_shader("res/shader/frontend.vs.glsl", "res/shader/frontend.fs.glsl"));
}


static void load_sprites() {
  auto& sprites = resources.sprites;
  auto filter = ntf::tex_filter::nearest;
  auto wrap   = ntf::tex_wrap::repeat;

  uint8_t pixels[] = {0x0, 0x0, 0x0, 0xFE, 0x0, 0xFE, 0x0, 0x0, 0xFE, 0x0, 0xFE, 0x00};
  ntf::texture2d tex{&pixels[0], ivec2{2, 2}, ntf::tex_format::rgb};
  tex.set_filter(ntf::tex_filter::nearest);
  tex.set_wrap(ntf::tex_wrap::repeat);

  sprites.emplace("default", std::move(tex));
  sprites.emplace("enemies", 
                  ntf::load_spritesheet("res/spritesheet/enemies.json", filter, wrap));
  sprites.emplace("effects", 
                  ntf::load_spritesheet("res/spritesheet/effects.json", filter, wrap));
  sprites.emplace("chara", 
                  ntf::load_spritesheet("res/spritesheet/chara.json", filter, wrap));
}

static void load_fonts() {
  auto& fonts = resources.fonts;

  fonts.emplace("arial", ntf::load_font("res/fonts/arial.ttf"));
}


namespace res {

void init() {
  load_shaders();
  load_sprites();
  load_fonts();
}


shader::shader(std::string_view name) :
  _shader(resources.shaders.id(name)) {}
const ntf::shader_program& shader::get() const {
  assert(valid() && "Invalid shader");
  return resources.shaders[_shader];
}
bool shader::valid() const {
  return resources.shaders.has(_shader);
}


sprite_atlas::sprite_atlas(std::string_view name) : 
  _atlas(resources.sprites.id(name)) {}
const ntf::texture_atlas& sprite_atlas::get() const {
  assert(valid() && "Invalid atlas");
  return resources.sprites[_atlas];
}
bool sprite_atlas::valid() const {
  return resources.sprites.has(_atlas);
}
sprite sprite_atlas::at(ntf::texture_atlas::texture tex) const {
  return sprite{*this, tex};
}


const ntf::texture_atlas::texture_meta& sprite::meta() const {
  assert(valid() && "Invalid sprite");
  return _atlas.get()[_tex];
}
const ntf::texture2d& sprite::tex() const {
  assert(valid() && "Invalid sprite");
  return _atlas.get().tex();
}
bool sprite::valid() const {
  return _atlas.valid() && _atlas.get().has(_tex);
}


font::font(std::string_view name) :
  _font(resources.fonts.id(name)) {}
const ntf::font& font::get() const {
  assert(valid() && "Invalid font");
  return resources.fonts[_font];
}
bool font::valid() const {
  return resources.fonts.has(_font);
}

} // namespace res

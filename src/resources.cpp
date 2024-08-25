#include "resources.hpp"

static struct {
  ntf::resource_pool<renderer::shader_program> shaders;
  ntf::resource_pool<renderer::font> fonts;
  ntf::resource_pool<ntf::texture_atlas<renderer::texture2d>, 
    ntf::texture_atlas<renderer::texture2d>::data_type> sprites;
} resources;

static void load_shaders() {
  auto& shaders = resources.shaders;

  auto loader = [](std::string_view vert_path, std::string_view frag_path) -> renderer::shader_program {
    renderer::shader_program prog;
    renderer::shader_program::loader prog_loader;

    auto vert_src = ntf::file_contents(vert_path.data());
    auto frag_src = ntf::file_contents(frag_path.data());

    try {
      prog = prog_loader(vert_src, frag_src);
    } catch(...) {
      ntf::log::error("[resources::load_shader] Failed to link shader");
      throw;
    }

    return prog;
  };

  shaders.emplace("sprite", loader("res/shader/sprite.vs.glsl", "res/shader/sprite.fs.glsl"));
  shaders.emplace("font", loader("res/shader/font.vs.glsl", "res/shader/font.fs.glsl"));
  shaders.emplace("framebuffer", loader("res/shader/framebuffer.vs.glsl", "res/shader/framebuffer.fs.glsl"));
  shaders.emplace("frontend", loader("res/shader/frontend.vs.glsl", "res/shader/frontend.fs.glsl"));
}

static void load_sprites() {
  auto& sprites = resources.sprites;
  auto filter = ntf::tex_filter::nearest;
  auto wrap   = ntf::tex_wrap::repeat;

  uint8_t pixels[] = {0x0, 0x0, 0x0, 0xFE, 0x0, 0xFE, 0x0, 0x0, 0xFE, 0x0, 0xFE, 0x00};
  renderer::texture2d tex{&pixels[0], ivec2{2, 2}, ntf::tex_format::rgb};
  tex.set_filter(ntf::tex_filter::nearest);
  tex.set_wrap(ntf::tex_wrap::repeat);

  ntf::texture_atlas<renderer::texture2d>::data_type::loader loader;

  sprites.emplace("default", std::move(tex));
  sprites.emplace("enemies", loader("res/spritesheet/enemies.json", filter, wrap));
  sprites.emplace("effects", loader("res/spritesheet/effects.json", filter, wrap));
  sprites.emplace("chara", loader("res/spritesheet/chara.json", filter, wrap));
}

static void load_fonts() {
  auto& fonts = resources.fonts;

  ntf::font_data<renderer::font>::loader loader;

  fonts.emplace("arial", loader("res/fonts/arial.ttf"));
}


namespace res {

void init() {
  load_shaders();
  load_sprites();
  load_fonts();
}

void destroy() {
  resources.fonts.clear();
  resources.sprites.clear();
  resources.shaders.clear();
}

shader::shader(std::string_view name) :
  _shader(resources.shaders.id(name)) {}

const renderer::shader_program& shader::get() const {
  assert(valid() && "Invalid shader");
  return resources.shaders[_shader];
}

bool shader::valid() const {
  return resources.shaders.has(_shader);
}


sprite_atlas::sprite_atlas(std::string_view name) : 
  _atlas(resources.sprites.id(name)) {}

const ntf::texture_atlas<renderer::texture2d>& sprite_atlas::get() const {
  assert(valid() && "Invalid atlas");
  return resources.sprites[_atlas];
}

bool sprite_atlas::valid() const {
  return resources.sprites.has(_atlas);
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
  _font(resources.fonts.id(name)) {}

const renderer::font& font::get() const {
  assert(valid() && "Invalid font");
  return resources.fonts[_font];
}

bool font::valid() const {
  return resources.fonts.has(_font);
}

} // namespace res

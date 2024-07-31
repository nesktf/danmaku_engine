#include "resources.hpp"

#include <shogle/res/util.hpp>
#include <shogle/core/log.hpp>

static struct {
  ntf::strmap<ntf::shogle::spritesheet> sprites;
  ntf::strmap<ntf::shogle::shader_program> shaders;
  ntf::strmap<ntf::shogle::font> fonts;
} resources;

static ntf::shogle::shader_program load_shader(std::string_view vert_path, std::string_view frag_path) {
  ntf::shogle::shader_program prog;

  auto vert_src = ntf::shogle::file_contents(vert_path.data());
  auto frag_src = ntf::shogle::file_contents(frag_path.data());

  try {
    prog = ntf::shogle::load_shader_program(vert_src, frag_src);
  } catch(...) {
    ntf::log::error("[resources::load_shader] Failed to link shader");
    throw;
  }

  return prog;
}

static void load_shaders() {
  auto& shaders = resources.shaders;
  shaders.emplace(std::make_pair("sprite", 
    load_shader("res/shaders/sprite.vs.glsl", "res/shaders/sprite.fs.glsl")));
  shaders.emplace(std::make_pair("font", 
    load_shader("res/shaders/font.vs.glsl", "res/shaders/font.fs.glsl")));
  shaders.emplace(std::make_pair("framebuffer", 
    load_shader("res/shaders/framebuffer.vs.glsl", "res/shaders/framebuffer.fs.glsl")));
}

static void load_fonts() {

}

static void load_sprites() {
  auto& sprites = resources.sprites;
  auto filter = ntf::shogle::tex_filter::nearest;
  auto wrap   = ntf::shogle::tex_wrap::repeat;

  sprites.emplace(std::make_pair("enemies", 
    ntf::shogle::load_spritesheet("res/spritesheets/enemies.json", filter, wrap)));
  sprites.emplace(std::make_pair("effects", 
    ntf::shogle::load_spritesheet("res/spritesheets/effects.json", filter, wrap)));
  sprites.emplace(std::make_pair("chara", 
    ntf::shogle::load_spritesheet("res/spritesheets/chara.json", filter, wrap)));
}

void res::init() {
  load_shaders();
  load_sprites();
  load_fonts();
}

const spritesheet& res::spritesheet(std::string_view name) {
  return resources.sprites.at(name.data());
}

const shader_program& res::shader(std::string_view name) {
  return resources.shaders.at(name.data());
}

const font& res::font(std::string_view name) {
  return resources.fonts.at(name.data());
}


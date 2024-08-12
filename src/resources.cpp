#include "resources.hpp"

#include <shogle/res/util.hpp>
#include <shogle/core/log.hpp>

static struct {
  ntf::resource_pool<ntf::shader_program> shaders;
  ntf::resource_pool<ntf::spritesheet, ntf::spritesheet_data> sprites;
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
}

static void load_sprites() {
  auto& sprites = resources.sprites;
  auto filter = ntf::tex_filter::nearest;
  auto wrap   = ntf::tex_wrap::repeat;

  sprites.emplace("enemies", 
                  ntf::load_spritesheet("res/spritesheet/enemies.json", filter, wrap));
  sprites.emplace("effects", 
                  ntf::load_spritesheet("res/spritesheet/effects.json", filter, wrap));
  sprites.emplace("chara", 
                  ntf::load_spritesheet("res/spritesheet/chara.json", filter, wrap));
}

void res::init() {
  load_shaders();
  load_sprites();
}

res::spritesheet_id res::spritesheet_index(std::string_view name) {
  return resources.sprites.id(name);
}

const ntf::spritesheet& res::spritesheet_at(res::spritesheet_id sheet) {
  return resources.sprites.at(sheet);
}

const ntf::spritesheet& res::spritesheet_at(std::string_view name) {
  return resources.sprites.at(name);
}

const ntf::spritesheet::sprite_data& res::sprite_data_at(res::sprite_id sprite) {
  const auto& sheet = resources.sprites.at(sprite.sheet);
  return sheet[sprite.index];
}

res::shader_id res::shader_index(std::string_view name) {
  return resources.shaders.id(name);
}

const ntf::shader_program& res::shader_at(res::shader_id shader) {
  return resources.shaders.at(shader);
}

const ntf::shader_program& res::shader_at(std::string_view name) {
  return resources.shaders.at(name);
}

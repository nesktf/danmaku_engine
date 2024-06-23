#pragma once

#include <shogle/shogle.hpp>

#include <shogle/res/spritesheet.hpp>
#include <shogle/res/model.hpp>

#include <shogle/res/shader/sprite.hpp>
#include <shogle/res/shader/model.hpp>

#include <shogle/res/meshes.hpp>

namespace ntf {

struct world_object {
  world_object(shogle::sprite* sprite_) :
    sprite(sprite_) {}
  shogle::transform2d transform{};
  shogle::sprite* sprite{};
  color4 color{1.0f};
  size_t index{0};
  vec2 vel {0.0f};
  bool del {false};
  float ang_speed {0.0f};
};

class sprite_renderer {
public:
  sprite_renderer() : _quad(shogle::load_quad(shogle::quad_type::normal2d)) {}

public:
  void operator()(const shogle::camera2d& cam, const world_object& obj) {
    _shader.set_proj(cam.proj())
      .set_view(cam.view())
      .set_transform(obj.transform.transf())
      .set_color(obj.color)
      .set_tex_offset(obj.sprite->tex_offset(obj.index))
      .bind_texture(obj.sprite->tex())
      .draw(_quad);
  }

private:
  shogle::mesh _quad{};
  shogle::sprite_shader _shader{};
};

class model_renderer {
public:
  model_renderer() = default;

public:
  void operator()(const shogle::camera3d& cam, const shogle::transform3d& transform, shogle::model& model) {
    for (auto& [name, tex_mesh] : model) {
      _shader.set_proj(cam.proj())
        .set_view(cam.view())
        .set_model(transform.transf())
        .bind_diffuse(tex_mesh[shogle::material_type::diffuse])
        .draw(tex_mesh.get_mesh());
    }
  }

private:
  shogle::model_shader _shader{};
};

} // namespace ntf

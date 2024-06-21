#pragma once

#include <shogle/shogle.hpp>

#include <shogle/res/spritesheet.hpp>
#include <shogle/res/model.hpp>

#include <shogle/res/shader/sprite.hpp>
#include <shogle/res/shader/model.hpp>

#include <shogle/res/mesh/quad.hpp>

namespace ntf {

class sprite_renderer {
public:
  sprite_renderer(shogle::sprite_shader& shader, shogle::sprite& sprite, shogle::quad& quad) :
    _shader(shader), _sprite(sprite), _quad(quad) {}

public:
  void operator()(const shogle::camera2d& cam, const shogle::transform2d& transform, size_t index) {
    _shader.set_proj(cam.proj())
      .set_view(mat4{1.0f})
      .set_transform(transform.transf())
      .set_color(color4{1.0f})
      .set_tex_offset(_sprite.tex_offset(index))
      .bind_texture(_sprite.tex())
      .draw(_quad.mesh());
  }

private:
  shogle::sprite_shader& _shader;
  shogle::sprite& _sprite;
  shogle::quad& _quad;
};

class model_renderer {
public:
  model_renderer(shogle::model_shader& shader, shogle::model& model) :
    _shader(shader), _model(model) {}

public:
  void operator()(const shogle::camera3d& cam, const shogle::transform3d& transform) {
    for (auto& mod_mesh : _model) {
      _shader.set_proj(cam.proj())
        .set_view(mat4{1.0f})
        .set_model(transform.transf())
        .bind_diffuse(mod_mesh.find_material(shogle::material_type::diffuse))
        .draw(mod_mesh.mesh());
    }
  }

private:
  shogle::model_shader& _shader;
  shogle::model& _model;
};

} // namespace ntf

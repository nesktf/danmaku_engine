#include "renderer.hpp"
#include "danmaku.hpp"

namespace ntf {

sprite_renderer::sprite_renderer() : 
  _quad(shogle::load_quad(shogle::quad_type::normal2d)) {}

void sprite_renderer::operator()(const shogle::camera2d& cam, const entity2d& obj) {
  _shader.set_proj(cam.proj())
    .set_view(cam.view())
    .set_transform(obj.transform.mat())
    .set_color(obj.color)
    .set_tex_offset(obj.sprite->tex_offset(obj.index))
    .bind_texture(obj.sprite->tex())
    .draw(_quad);
}

void model_renderer::operator()(const shogle::camera3d& cam, const shogle::transform3d& transform, shogle::model& model) {
  for (auto& [name, tex_mesh] : model) {
    _shader.set_proj(cam.proj())
      .set_view(cam.view())
      .set_transform(transform.mat())
      .bind_diffuse(tex_mesh[shogle::material_type::diffuse])
      .draw(tex_mesh.get_mesh());
  }
}

} // namespace ntf

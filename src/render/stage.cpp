#include "./stage.hpp"
#include "./instance.hpp"
#include <ntfstl/utility.hpp>

namespace okuu::render {

stage_viewport::stage_viewport(u32 width, u32 height, u32 xpos, u32 ypos,
                               shogle::texture2d&& fb_tex, shogle::framebuffer&& fb) :
    _fb_tex{std::move(fb_tex)}, _fb{std::move(fb)}, _width{width}, _height{height}, _xpos{xpos},
    _ypos{ypos} {}

expect<stage_viewport> stage_viewport::create(u32 width, u32 height, u32 xpos, u32 ypos) {
  NTF_ASSERT(g_renderer.has_value());
  return create_framebuffer(width, height).transform([&](auto&& fb_pair) -> stage_viewport {
    auto&& [fb_tex, fb] = std::forward<decltype(fb_pair)>(fb_pair);
    return {width, height, xpos, ypos, std::move(fb_tex), std::move(fb)};
  });
}

shogle::texture_binding stage_viewport::tex_binds(u32 sampler) const {
  return {
    .texture = _fb_tex,
    .sampler = sampler,
  };
}

std::pair<u32, u32> stage_viewport::extent() const {
  auto ext = _fb_tex.extent();
  return std::make_pair(ext.x, ext.y);
}

std::pair<u32, u32> stage_viewport::pos() const {
  return std::make_pair(_xpos, _ypos);
}

mat4 stage_viewport::transform() const {
  // The stage viewport works with screen space coordinates
  // Each pixel should map 1:1 to the window viewport
  auto [width, height] = extent();
  auto transf = shogle::transform2d<float>{}.scale(width, height).pos(_xpos, _ypos);
  const auto ret = transf.world();
  return ret;
}

mat4 stage_viewport::proj() const {

  auto [width, height] = extent();
  return glm::ortho(0.f, (f32)width, (f32)height, 0.f, -10.f, 1.f);
}

mat4 stage_viewport::view() const {
  auto sz = static_cast<vec2>(_fb_tex.extent());
  return shogle::build_view_matrix(vec2{0, 0}, sz * .5f, vec2{1.f, 1.f}, vec3{0.f});
}

void render_viewport(shogle::quad_mesh& quad, stage_viewport& viewport) {
  NTF_ASSERT(g_renderer.has_value());
  auto fb = shogle::framebuffer::get_default(g_renderer->ctx);
  auto& pip = g_renderer->stage_pip;

  auto loc_model = pip.uniform_location("model").value();
  auto loc_proj = pip.uniform_location("proj").value();
  auto loc_sampler = pip.uniform_location("fb_sampler").value();

  i32 sampler = 0;
  auto tbind = viewport.tex_binds(sampler);

  const auto proj = glm::ortho(0.f, 1280.f, 720.f, 0.f, -10.f, 1.f);

  const mat4 model = viewport.transform();
  shogle::uniform_const unifs[] = {
    shogle::format_uniform_const(loc_model, model),
    shogle::format_uniform_const(loc_proj, proj),
    shogle::format_uniform_const(loc_sampler, sampler),
  };

  g_renderer->ctx.submit_render_command({
    .target = fb,
    .pipeline = pip,
    .buffers = quad.bindings(),
    .textures = {tbind},
    .consts = unifs,
    .opts =
      {
        .vertex_count = 6,
        .vertex_offset = 0,
        .index_offset = 0,
        .instances = 0,
      },
    .sort_group = 0,
    .render_callback = {},
  });
}

void stage_renderer::enqueue_sprite(const sprite_render_data& sprite_data) {
  NTF_ASSERT(_sprite_instances < _max_instances); // fix this later

  const auto setup_sprite_samplers = [&]() -> i32 {
    u32 i = 0;
    for (; i < _active_texes; ++i) {
      auto& tex = _tex_binds[i];
      NTF_ASSERT(tex.texture != nullptr);
      if (tex.texture == sprite_data.texture.get()) {
        return static_cast<i32>(i);
      }
    }

    NTF_ASSERT(i != _tex_binds.size(), "Over the texture binding limit :c");
    _tex_binds[i].texture = sprite_data.texture;
    ++_active_texes;
    return static_cast<i32>(i);
  };

  const sprite_shader_data shader_data{
    .transform = sprite_data.transform,
    .view = _viewport.view(),
    .proj = _viewport.proj(),
    .sampler = setup_sprite_samplers(),
    .ticks = static_cast<i32>(sprite_data.ticks),
    .uv_lin_x = sprite_data.uvs.x_lin,
    .uv_lin_y = sprite_data.uvs.y_lin,
    .uv_con_x = sprite_data.uvs.x_con,
    .uv_con_y = sprite_data.uvs.y_con,
  };

  _sprite_buffer.upload(shader_data, _sprite_instances * sizeof(shader_data));
  ++_sprite_instances;
}

void stage_renderer::reset_instances() {
  // Reset texture bindings
  std::memset(_tex_binds.data(), 0, _tex_binds.size());
  _active_texes = 0u;
  _sprite_instances = 0u;
}

ntf::cspan<shogle::texture_binding> stage_renderer::tex_binds() const {
  if (_active_texes == 0u) {
    return {};
  }
  return {_tex_binds.data(), _active_texes};
}

ntf::cspan<shogle::shader_binding> stage_renderer::shader_binds() const {
  return {_sprite_buffer_binds};
}

void render_stage(stage_renderer& stage) {
  NTF_ASSERT(g_renderer.has_value());
  auto& quad = g_renderer->quad;
  auto& pipeline = g_renderer->sprite_pipeline;

  auto& vp = stage.viewport();
  g_renderer->ctx.submit_render_command({
    .target = vp.framebuffer(),
    .pipeline = pipeline,
    .buffers = quad.bindings(stage.shader_binds()),
    .textures = stage.tex_binds(),
    .consts = {},
    .opts =
      {
        .vertex_count = 6,
        .vertex_offset = 0,
        .index_offset = 0,
        .instances = stage.sprite_instances(),
      },
    .sort_group = 0,
    .render_callback = {},
  });

  stage.reset_instances();
}

} // namespace okuu::render

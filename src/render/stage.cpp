#include "./stage.hpp"
#include "./instance.hpp"
#include <ntfstl/utility.hpp>

namespace okuu::render {

stage_viewport::stage_viewport(shogle::texture2d&& fb_tex, shogle::framebuffer&& fb,
                               shogle::pipeline&& pip, shogle::shader_storage_buffer&& ssbo,
                               u32 xpos, u32 ypos) :
    _fb_tex{std::move(fb_tex)},
    _fb{std::move(fb)}, _pip{std::move(pip)}, _ssbo{std::move(ssbo)}, _xpos{xpos}, _ypos{ypos} {
  const vec2 sz = (vec2)_fb_tex.extent();

  _unif.proj = glm::ortho(0.f, sz.x, sz.y, 0.f, -10.f, 1.f);
  _unif.view = shogle::build_view_matrix(vec2{0, 0}, sz * .5f, vec2{1.f, 1.f}, vec3{0.f});
  _ssbo.upload(_unif);
}

stage_viewport stage_viewport::create(u32 width, u32 height, u32 xpos, u32 ypos) {
  NTF_ASSERT(g_renderer.has_value());

  const shogle::typed_texture_desc fb_tex_desc{
    .format = shogle::image_format::rgba8u,
    .sampler = shogle::texture_sampler::nearest,
    .addressing = shogle::texture_addressing::repeat,
    .extent = {width, height, 1},
    .layers = 1u,
    .levels = 1u,
    .data = nullptr,
  };

  auto fb_tex = shogle::texture2d::create(g_renderer->ctx, fb_tex_desc).value();

  const shogle::fbo_image fb_img[] = {{
    .texture = fb_tex,
    .layer = 0,
    .level = 0,
  }};
  const shogle::fbo_image_desc fb_desc{
    .extent = {width, height},
    .viewport = {0, 0, width, height},
    .clear_color{.3f, .3f, .3f, 1.f},
    .clear_flags = shogle::clear_flag::color_depth,
    .test_buffer = shogle::fbo_buffer::depth24u_stencil8u,
    .images = fb_img,
  };
  auto fb = shogle::framebuffer::create(g_renderer->ctx, fb_desc).value();
  auto pip = create_pipeline().value();

  const shogle::typed_buffer_desc ssbo_desc{
    .flags = shogle::buffer_flag::dynamic_storage,
    .size = sizeof(stage_uniforms),
    .data = nullptr,
  };

  auto ssbo = shogle::shader_storage_buffer::create(g_renderer->ctx, ssbo_desc).value();

  return {std::move(fb_tex), std::move(fb), std::move(pip), std::move(ssbo), xpos, ypos};
}

void stage_viewport::render() {
  NTF_ASSERT(g_renderer.has_value());
  auto fb = shogle::framebuffer::get_default(g_renderer->ctx);
  auto& pip = g_renderer->stage_pip;

  auto loc_model = pip.uniform_location("model").value();
  auto loc_proj = pip.uniform_location("proj").value();
  auto loc_sampler = pip.uniform_location("fb_sampler").value();

  const auto proj = glm::ortho(0.f, 1280.f, 720.f, 0.f, -10.f, 1.f);
  auto sz = _fb_tex.extent();
  auto transf = shogle::transform2d<float>{}.scale(sz.x, sz.y).pos(_xpos, _ypos);

  const mat4 model = transf.world();
  shogle::uniform_const unifs[] = {
    shogle::format_uniform_const(loc_model, model),
    shogle::format_uniform_const(loc_proj, proj),
    shogle::format_uniform_const(loc_sampler, 0),
  };

  const shogle::texture_binding tbind{
    .texture = _fb_tex,
    .sampler = 0,
  };

  g_renderer->ctx.submit_render_command({
    .target = fb,
    .pipeline = pip,
    .buffers = g_renderer->quad.bindings(),
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

void render_sprites(stage_viewport& stage, std::vector<sprite_command>& cmds) {
  NTF_ASSERT(g_renderer.has_value());
  auto& ctx = g_renderer->ctx;

  for (const auto& cmd : cmds) {
    NTF_ASSERT(cmd.pipeline);
    NTF_ASSERT(cmd.texture);
    const shogle::texture_binding tbind{
      .texture = *cmd.texture,
      .sampler = 0,
    };
    shogle::buffer_binding binds = g_renderer->quad.bindings();
    if (cmd.ssbo_bind) {
      binds.shader = {*cmd.ssbo_bind};
    }

    ctx.submit_render_command({
      .target = stage.framebuffer(),
      .pipeline = *cmd.pipeline,
      .buffers = binds,
      .textures = {tbind},
      .consts = {},
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
}

sprite::sprite(shogle::texture2d&& tex) : _tex{std::move(tex)} {}

sprite sprite::from_spritesheet(const chima::spritesheet& sheet) {
  NTF_ASSERT(g_renderer.has_value());

  const auto [width, height] = sheet.atlas_extent();
  const auto atlas_bitmap = sheet.atlas_data();

  NTF_ASSERT(sheet.get_image().depth == chima_image_depth::CHIMA_DEPTH_8U);
  NTF_ASSERT(sheet.get_image().channels == 4u);

  const shogle::image_data image{
    .bitmap = atlas_bitmap,
    .format = shogle::image_format::rgba8u,
    .alignment = 1u,
    .extent = {width, height, 1},
    .offset = {0, 0, 0},
    .layer = 0u,
    .level = 0u,
  };
  const shogle::texture_data data{
    .images = {image},
    .generate_mipmaps = false,
  };
  const shogle::typed_texture_desc desc{
    .format = shogle::image_format::rgba8u,
    .sampler = shogle::texture_sampler::nearest,
    .addressing = shogle::texture_addressing::repeat,
    .extent = {width, height, 1},
    .layers = 1u,
    .levels = 1u,
    .data = data,
  };

  auto spr = shogle::texture2d::create(g_renderer->ctx, desc)
               .transform_error([](auto&& err) -> std::string { return err.what(); })
               .value();

  return {std::move(spr)};
}

void sprite::render(stage_viewport& vp) {
  NTF_ASSERT(g_renderer.has_value());
  auto& pip = g_renderer->stage_pip;

  auto loc_model = pip.uniform_location("model").value();
  auto loc_proj = pip.uniform_location("proj").value();
  auto loc_sampler = pip.uniform_location("fb_sampler").value();

  auto [width, height] = vp.extent();
  const auto proj = glm::ortho(0.f, ntf::implicit_cast<float>(width),
                               ntf::implicit_cast<float>(height), 0.f, -10.f, 1.f);
  auto transf =
    shogle::transform2d<float>{}.scale(600, -600).pos((float)width / 2, (float)height / 2);

  const mat4 model = transf.world();
  shogle::uniform_const unifs[] = {
    shogle::format_uniform_const(loc_model, model),
    shogle::format_uniform_const(loc_proj, proj),
    shogle::format_uniform_const(loc_sampler, 0),
  };

  const shogle::texture_binding tbind{
    .texture = _tex,
    .sampler = 0,
  };

  g_renderer->ctx.submit_render_command({
    .target = vp.framebuffer(),
    .pipeline = pip,
    .buffers = g_renderer->quad.bindings(),
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

} // namespace okuu::render

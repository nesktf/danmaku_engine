#include "./instance.hpp"
#include "./shader_src.hpp"
#include <ntfstl/logger.hpp>

namespace okuu::render {

namespace {

static constexpr u32 SPRITE_BATCH_SIZE = 1024u;

std::string shogle_to_str(shogle::render_error&& err) {
  return err.what();
}

expect<shogle::texture2d> make_tex(shogle::context_view ctx, u32 width, u32 height,
                                   const void* data) {
  const auto make_thing = [&](ntf::weak_cptr<shogle::texture_data> tex_data) {
    shogle::typed_texture_desc desc{
      .format = shogle::image_format::rgba8u,
      .sampler = shogle::texture_sampler::nearest,
      .addressing = shogle::texture_addressing::repeat,
      .extent = {width, height, 1},
      .layers = 1u,
      .levels = 1u,
      .data = tex_data,
    };
    return shogle::texture2d::create(ctx, desc).transform_error(shogle_to_str);
  };

  if (data) {
    const shogle::image_data image{
      .bitmap = data,
      .format = shogle::image_format::rgba8u,
      .alignment = 4u,
      .extent = {width, height, 1},
      .offset = {0, 0, 0},
      .layer = 0u,
      .level = 0u,
    };
    const shogle::texture_data data{
      .images = {image},
      .generate_mipmaps = false,
    };
    return make_thing(data);
  } else {
    return make_thing({});
  }
}

expect<std::pair<shogle::texture2d, shogle::framebuffer>> make_fb(shogle::context_view ctx,
                                                                  u32 width, u32 height) {
  using lambda_ret = expect<std::pair<shogle::texture2d, shogle::framebuffer>>;
  const auto make_thing = [&](shogle::texture2d&& tex) -> lambda_ret {
    const shogle::fbo_image image{
      .texture = tex,
      .layer = 0,
      .level = 0,
    };
    const shogle::fbo_image_desc fb_desc{
      .extent = {width, height},
      .viewport = {0, 0, width, height},
      .clear_color = {.3f, .3f, .3f, 1.f},
      .clear_flags = shogle::clear_flag::color_depth,
      .test_buffer = shogle::fbo_buffer::depth24u_stencil8u,
      .images = {image},
    };
    auto fb = shogle::framebuffer::create(ctx, fb_desc);
    if (!fb) {
      return {ntf::unexpect, shogle_to_str(std::move(fb.error()))};
    }
    return {ntf::in_place, std::move(tex), std::move(*fb)};
  };

  return make_tex(ctx, width, height, nullptr).and_then(make_thing);
}

expect<shogle::pipeline> make_pip(shogle::context_view ctx, shogle::vertex_shader_view vert,
                                  shogle::fragment_shader_view frag,
                                  ntf::cspan<shogle::attribute_binding> attribs) {
  const shogle::blend_opts blending{
    .mode = shogle::blend_mode::add,
    .src_factor = shogle::blend_factor::src_alpha,
    .dst_factor = shogle::blend_factor::inv_src_alpha,
    .color = {0.f, 0.f, 0.f, 0.f},
  };
  const shogle::depth_test_opts depth_test{
    .func = shogle::test_func::less,
    .near_bound = 0.f,
    .far_bound = 1.f,
  };

  const shogle::shader_t stages_fb[] = {vert.get(), frag.get()};
  const shogle::pipeline_desc pip_desc{
    .attributes = {attribs.data(), attribs.size()},
    .stages = stages_fb,
    .primitive = shogle::primitive_mode::triangles,
    .poly_mode = shogle::polygon_mode::fill,
    .poly_width = 1.f,
    .tests =
      {
        .stencil_test = nullptr,
        .depth_test = depth_test,
        .scissor_test = nullptr,
        .face_culling = nullptr,
        .blending = blending,
      },
  };
  return shogle::pipeline::create(ctx, pip_desc).transform_error(shogle_to_str);
}

expect<shogle::buffer> make_buffer(shogle::context_view ctx, shogle::buffer_type type, size_t size,
                                   const void* data) {
  using lambda_ret = expect<shogle::buffer>;
  const auto make_thing = [&](ntf::weak_cptr<shogle::buffer_data> buff_data) -> lambda_ret {
    const shogle::buffer_desc desc{
      .type = type,
      .flags = shogle::buffer_flag::dynamic_storage,
      .size = size,
      .data = buff_data,
    };
    return shogle::buffer::create(ctx, desc).transform_error(shogle_to_str);
  };
  if (data) {
    const shogle::buffer_data buff_data{
      .data = data,
      .size = size,
      .offset = 0u,
    };
    return make_thing(buff_data);
  } else {
    return make_thing({});
  }
}

template<u32 tex_extent>
[[maybe_unused]] constexpr auto missing_albedo_bitmap = [] {
  std::array<u8, 4u * tex_extent * tex_extent> out;
  const u8 pixels[]{
    0x00, 0x00, 0x00, 0xFF, // black
    0xFE, 0x00, 0xFE, 0xFF, // pink
    0x00, 0x00, 0x00, 0xFF, // black again
  };

  for (u32 i = 0; i < tex_extent; ++i) {
    const u8* start = i % 2 == 0 ? &pixels[0] : &pixels[4]; // Start row with a different color
    u32 pos = 0;
    for (u32 j = 0; j < tex_extent; ++j) {
      pos = (pos + 4) % 8;
      for (u32 k = 0; k < 4; ++k) {
        out[(4 * i * tex_extent) + (4 * j) + k] = start[pos + k];
      }
    }
  }

  return out;
}();

template<u32 tex_extent = 16u>
expect<shogle::texture2d> make_missing_albedo(shogle::context_view ctx) {
  return make_tex(ctx, tex_extent, tex_extent, missing_albedo_bitmap<tex_extent>.data());
}

sprite_renderer make_sprite_renderer(shogle::context_view ctx, u32 instances,
                                     size_t elem_buffer_sz) {
  return make_buffer(ctx, shogle::buffer_type::shader_storage, instances * elem_buffer_sz, nullptr)
    .transform([](shogle::buffer&& buff) {
      return shogle::to_typed<shogle::buffer_type::shader_storage>(std::move(buff));
    })
    .and_then([=](shogle::shader_storage_buffer&& ssbo) -> expect<sprite_renderer> {
      auto quad = shogle::quad_mesh::create(ctx);
      if (!quad) {
        return {ntf::unexpect, shogle_to_str(std::move(quad.error()))};
      }
      return {ntf::in_place, instances, std::move(ssbo), std::move(*quad)};
    })
    .value();
}

} // namespace

ntf::nullable<okuu_render_ctx> g_renderer;

okuu_render_ctx::okuu_render_ctx(shogle::window&& win_, shogle::context&& ctx_,
                                 shader_data&& shaders_, sprite_renderer&& sprite_ren_,
                                 shogle::pipeline&& stage_pip_, shogle::texture2d&& base_fb_tex_,
                                 shogle::framebuffer&& base_fb_, shogle::pipeline&& back_pip_) :
    win{std::move(win_)}, ctx{std::move(ctx_)}, shaders{std::move(shaders_)},
    sprite_ren{std::move(sprite_ren_)}, stage_pip{std::move(stage_pip_)},
    base_fb_tex{std::move(base_fb_tex_)}, base_fb{std::move(base_fb_)},
    back_pip{std::move(back_pip_)}, missing_tex{make_missing_albedo<4>(ctx).value()} {}

[[nodiscard]] singleton_handle init() {
  const u32 win_width = 1280;
  const u32 win_height = 720;

  NTF_ASSERT(!g_renderer.has_value());
  const shogle::win_x11_params x11{
    .class_name = "okuu_engine",
    .instance_name = "okuu_engine",
  };
  const shogle::win_gl_params win_gl{
    .ver_major = 4,
    .ver_minor = 6,
    .swap_interval = 1,
    .fb_msaa_level = 8,
    .fb_buffer = shogle::fbo_buffer::depth24u_stencil8u,
    .fb_use_alpha = false,
  };
  const shogle::win_params win_params{
    .width = win_width,
    .height = win_height,
    .title = "test",
    .attrib = shogle::win_attrib::decorate | shogle::win_attrib::resizable,
    .renderer_api = shogle::context_api::opengl,
    .platform_params = &x11,
    .renderer_params = &win_gl,
  };
  auto win = shogle::window::create(win_params).value();

  const auto fb_clear = shogle::clear_flag::color_depth;
  const shogle::color4 fb_color{.3f, .3f, .3f, 1.f};
  const auto vp = shogle::uvec4{0, 0, win.fb_size()};
  const auto gl_params = shogle::window::make_gl_params(win);

  const shogle::context_params ctx_params{
    .ctx_params = &gl_params,
    .ctx_api = shogle::context_api::opengl,
    .fb_viewport = vp,
    .fb_clear_flags = fb_clear,
    .fb_clear_color = fb_color,
    .alloc = nullptr,
  };
  auto ctx = shogle::context::create(ctx_params).value();

  auto sprite_vert_shader = shogle::vertex_shader::create(ctx, {vert_sprite}).value();
  auto sprite_frag_shader = shogle::fragment_shader::create(ctx, {frag_sprite}).value();

  auto vert_fbo_shader = shogle::vertex_shader::create(ctx, {vert_fbo}).value();
  auto frag_fbo_shader = shogle::fragment_shader::create(ctx, {frag_fbo}).value();

  auto frag_back_shader = shogle::fragment_shader::create(ctx, {frag_back}).value();

  const auto attribs = shogle::pnt_vertex::aos_binding();
  auto pip_vp = make_pip(ctx, vert_fbo_shader, frag_fbo_shader, attribs).value();

  auto pip_back = make_pip(ctx, vert_fbo_shader, frag_back_shader, attribs).value();

  auto [fb_tex, fb] = make_fb(ctx, 1280, 720).value();

  auto sprite_ren = make_sprite_renderer(ctx, SPRITE_BATCH_SIZE, sizeof(sprite_shader_data));

  g_renderer.emplace(std::move(win), std::move(ctx),
                     shader_data{
                       .vert_sprite_generic = std::move(sprite_vert_shader),
                       .frag_sprite_generic = std::move(sprite_frag_shader),
                     },
                     std::move(sprite_ren), std::move(pip_vp), std::move(fb_tex), std::move(fb),
                     std::move(pip_back));
  NTF_ASSERT(g_renderer.has_value());
  g_renderer->win.set_viewport_callback([](auto&, uvec2 vp) {
    shogle::framebuffer::get_default(g_renderer->ctx).viewport({0.f, 0.f, vp.x, vp.y});
    g_renderer->viewport_event.fire(vp.x, vp.y);
  });
  return {};
}

expect<shogle::texture2d> create_texture(u32 width, u32 height, const void* data) {
  NTF_ASSERT(g_renderer.has_value());
  return make_tex(g_renderer->ctx, width, height, data);
}

expect<std::pair<shogle::texture2d, shogle::framebuffer>> create_framebuffer(u32 width,
                                                                             u32 height) {
  NTF_ASSERT(g_renderer.has_value());
  return make_fb(g_renderer->ctx, width, height);
}

singleton_handle::~singleton_handle() noexcept {
  NTF_ASSERT(g_renderer.has_value());
  g_renderer.reset();
}

shogle::window& window() {
  NTF_ASSERT(g_renderer.has_value());
  return g_renderer->win;
}

expect<shogle::pipeline> create_pipeline(std::string_view frag_src, pipeline_attrib attrib) {
  NTF_ASSERT(g_renderer.has_value());

  using lambda_ret = std::pair<ntf::cspan<shogle::attribute_binding>, shogle::vertex_shader_view>;
  const auto get_attrib = [&]() -> lambda_ret {
    const auto& shaders = g_renderer->shaders;

    static constexpr auto pnt_aos = shogle::pnt_vertex::aos_binding();
    switch (attrib) {
      case pipeline_attrib::sprite_generic: {
        ntf::cspan<shogle::attribute_binding> attr{pnt_aos.data(), pnt_aos.size()};
        return {attr, shaders.vert_sprite_generic};
      }
      default:
        return {};
    }
  };

  return shogle::fragment_shader::create(g_renderer->ctx, {frag_src})
    .transform_error(shogle_to_str)
    .and_then([&](shogle::fragment_shader&& fragment) {
      const auto [attrib, vertex] = get_attrib();
      return make_pip(g_renderer->ctx, vertex, fragment, attrib);
    });
}

expect<shogle::shader_storage_buffer> create_ssbo(size_t size, const void* data) {
  NTF_ASSERT(g_renderer.has_value());
  return make_buffer(g_renderer->ctx, shogle::buffer_type::shader_storage, size, data)
    .transform([](shogle::buffer&& buffer) -> shogle::shader_storage_buffer {
      auto ret = shogle::to_typed<shogle::buffer_type::shader_storage>(std::move(buffer));
      ntf::logger::debug("AAA {}", ret.size());
      return ret;
    });
}

void render_back(float t) {
  NTF_ASSERT(g_renderer.has_value());
  auto fb = shogle::framebuffer::get_default(g_renderer->ctx);
  auto& pip = g_renderer->back_pip;

  auto loc_proj = pip.uniform_location("proj").value();
  auto loc_model = pip.uniform_location("model").value();
  auto loc_sampler = pip.uniform_location("tex").value();
  auto loc_time = pip.uniform_location("time").value();

  const auto proj = glm::ortho(0.f, 1280.f, 720.f, 0.f, -10.f, 1.f);
  auto transf = shogle::transform2d<float>{}.scale(1280 * 2, 720 * 2);

  const mat4 model = transf.world();
  shogle::uniform_const unifs[] = {
    shogle::format_uniform_const(loc_model, model),
    shogle::format_uniform_const(loc_proj, proj),
    shogle::format_uniform_const(loc_time, t),
    shogle::format_uniform_const(loc_sampler, 0),
  };

  const shogle::texture_binding tbind{
    .texture = g_renderer->missing_tex,
    .sampler = 0,
  };

  g_renderer->ctx.submit_render_command({
    .target = fb,
    .pipeline = pip,
    .buffers = g_renderer->sprite_ren.quad().bindings(),
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

void sprite_renderer::enqueue_sprite(const sprite_render_data& sprite_data) {
  NTF_ASSERT(_sprite_instance_num < _max_instances);
  auto transform = shogle::transform2d<f32>{}
                     .pos(sprite_data.position)
                     .scale(sprite_data.scale)
                     .roll(sprite_data.rotation);

  i32 samplers[SHADER_SAMPLER_COUNT]{0};
  {
    u32 i = 0;
    for (; i < _active_texes; ++i) {
      auto& tex = _texes[i];
      if (tex.texture == sprite_data.texture.get()) {
        samplers[0] = i;
      }
    }
    if (i == _active_texes) {
    }
  }

  const sprite_shader_data shader_data{
    .transform = transform.world(),
    .samplers = {samplers[0], samplers[1], samplers[2], samplers[3]},
    .ticks = static_cast<i32>(sprite_data.ticks),
    .uv_lin_x = sprite_data.uv_lin.x,
    .uv_lin_y = sprite_data.uv_lin.y,
    .uv_con_x = sprite_data.uv_con.x,
    .uv_con_y = sprite_data.uv_con.y,
  };

  _buffer.upload(shader_data, _sprite_instance_num * sizeof(shader_data));
  ++_sprite_instance_num;
}

void sprite_renderer::render() {
  _sprite_instance_num = 0u;

  const shogle::shader_binding buff_bind{
    .buffer = _buffer,
    .binding = 0,
    .size = _buffer.size(),
    .offset = 0u,
  };

  g_renderer->ctx.submit_render_command({
    .target = {},
    .pipeline = {},
    .buffers = _quad.bindings({buff_bind}),
    .textures = {_texes.data(), _active_texes},
    .consts = {},
    .opts =
      {
        .vertex_count = 6,
        .vertex_offset = 0,
        .index_offset = 0,
        .instances = _sprite_instance_num,
      },
    .sort_group = 0,
    .render_callback = {},
  });

  // Reset textures
  std::memset(_texes.data(), 0, _texes.size());
}

void render_sprites(const std::vector<sprite_render_data>& sprites) {
  auto& cbs = g_renderer->render_cbs;

  cbs.clear();
  cbs.reserve(sprites.size());

  for (u32 i = 0; const auto& sprite : sprites) {
    cbs.emplace_back([&sprite](shogle::context_view) {
      std::invoke(sprite.on_render_buffer_write, sprite.buffer);
    });

    const shogle::shader_binding buff_bind{
      .buffer = sprite.buffer,
      .binding = 0,
      .size = sprite.buffer.size(),
      .offset = 0u,
    };
    const shogle::texture_binding tex_bind{
      .texture = sprite.texture,
      .sampler = 0,
    };
    g_renderer->ctx.submit_render_command({
      .target = sprite.target,
      .pipeline = sprite.pipeline,
      .buffers = g_renderer->sprite_ren.quad().bindings({buff_bind}),
      .textures = {tex_bind},
      .consts = {},
      .opts =
        {
          .vertex_count = 6,
          .vertex_offset = 0,
          .index_offset = 0,
          .instances = 0,
        },
      .sort_group = 0,
      .render_callback = {cbs[i]},
    });
    ++i;
  }
}

} // namespace okuu::render

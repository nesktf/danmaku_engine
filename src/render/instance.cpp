#include "./instance.hpp"
#include <ntfstl/logger.hpp>

namespace okuu::render {

namespace {

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

} // namespace

ntf::nullable<okuu_render_ctx> g_renderer;

okuu_render_ctx::okuu_render_ctx(shogle::window&& win_, shogle::context&& ctx_,
                                 shogle::quad_mesh&& quad_, shogle::texture2d&& missing_tex_,
                                 shogle::texture2d&& fb_tex_, shogle::framebuffer&& fb_,
                                 base_pipelines&& pips_) :
    win{std::move(win_)},
    ctx{std::move(ctx_)}, quad{std::move(quad_)}, missing_tex{std::move(missing_tex_)},
    fb_tex{std::move(fb_tex_)}, fb{std::move(fb_)}, pips{std::move(pips_)}, viewport_event{} {}

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

  auto quad = shogle::quad_mesh::create(ctx).value();
  auto pips = init_pipelines(ctx).value();
  auto missing_tex = make_missing_albedo(ctx).value();
  auto [fb_tex, fb] = make_fb(ctx, 1280, 720).value();

  g_renderer.emplace(std::move(win), std::move(ctx), std::move(quad), std::move(missing_tex),
                     std::move(fb_tex), std::move(fb), std::move(pips));
  NTF_ASSERT(g_renderer.has_value());
  g_renderer->win.set_viewport_callback([](auto&, uvec2 vp) {
    shogle::framebuffer::get_default(g_renderer->ctx).viewport({0.f, 0.f, vp.x, vp.y});
    g_renderer->viewport_event.trigger_event(vp.x, vp.y);
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

shogle::context_view shogle_ctx() {
  NTF_ASSERT(g_renderer.has_value());
  return g_renderer->ctx;
}

expect<shogle::shader_storage_buffer> create_ssbo(size_t size, const void* data) {
  NTF_ASSERT(g_renderer.has_value());
  return make_buffer(g_renderer->ctx, shogle::buffer_type::shader_storage, size, data)
    .transform([](shogle::buffer&& buffer) -> shogle::shader_storage_buffer {
      return shogle::to_typed<shogle::buffer_type::shader_storage>(std::move(buffer));
    });
}

void render_back(float t) {
  NTF_ASSERT(g_renderer.has_value());
  auto fb = shogle::framebuffer::get_default(g_renderer->ctx);
  auto& pip = g_renderer->pips.back;

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

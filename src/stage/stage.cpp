#define OKUU_SOL_IMPL
#include "../lua/sol.hpp"

#include "./stage.hpp"

#include "../render/instance.hpp"
#include "../render/stage.hpp"
#include <ntfstl/logger.hpp>

namespace okuu::stage {
namespace {

template<typename T>
concept renderable_entity = requires(const T ent) {
  { ent.sprite() } -> std::same_as<stage::entity_sprite>;
};

} // namespace

stage_scene::stage_scene(player_entity&& player, render::stage_renderer&& renderer) :
    _renderer{std::move(renderer)}, _projs{}, _bosses{}, _boss_count{}, _player{std::move(player)},
    _task_wait_ticks{0u}, _ticks{0u} {}

void stage_scene::render(double dt, double alpha, assets::asset_bundle& assets) {
  // The scene has to render the following (in order):
  // - The background
  // - The boss(es)
  // - The player
  // - The items
  // - The danmaku
  NTF_UNUSED(dt);
  NTF_UNUSED(alpha);

  const auto render_sprite = [&]<renderable_entity Ent>(Ent& entity) {
    const auto [atlas_handle, sprite] = entity.sprite();
    assets::sprite_atlas& atlas = assets.get_asset(atlas_handle);

    const auto [tex, uvs] = atlas.render_data(sprite);
    this->_renderer.enqueue_sprite({
      .transform = entity.transform(uvs),
      .texture = tex,
      .ticks = _ticks,
      .uvs = uvs,
      .color = {1.f, 1.f, 1.f, 1.f},
    });
  };

  for (u32 i = 0; i < _boss_count; ++i) {
    auto& boss = _bosses[i];
    if (!boss.is_active()) {
      continue;
    }
    render_sprite(boss);
  }

  render_sprite(_player);

  _projs.for_each([&](projectile_entity& proj) { render_sprite(proj); });

  auto render_target = shogle::framebuffer::get_default(render::g_renderer->ctx);
  render::render_stage(_renderer);
  render::render_viewport(_renderer.viewport(), render_target);
}

void stage_scene::tick() {
  for (u32 i = 0; i < _boss_count; ++i) {
    auto& boss = _bosses[i];
    if (!boss.is_active()) {
      continue;
    }
    boss.tick();
  }

  _projs.for_each([&](projectile_entity& proj) { proj.tick(); });

  _player.tick();
  ++_ticks;
}

u64 stage_scene::spawn_projectile(const projectile_args& args) {
  auto elem = _projs.request_elem(args);
  return elem.handle();
}

void stage_scene::kill_projectile(u64 handle) {
  _projs.return_elem(handle);
}

ntf::optional<u32> stage_scene::spawn_boss(const boss_args& args) {
  for (u32 i = 0; i < _boss_count; ++i) {
    auto& boss = _bosses[i];
    if (boss.is_active()) {
      continue;
    }
    boss.setup(args);
    ++_boss_count;
    return {ntf::in_place, i};
  }

  return {ntf::nullopt};
}

void stage_scene::kill_boss(u32 slot) {
  if (slot >= MAX_BOSSES) {
    return;
  }
  auto& boss = _bosses[slot];
  if (!boss.is_active()) {
    return;
  }
  boss.disable();
  --_boss_count;
}

boss_entity& stage_scene::get_boss(u32 slot) {
  NTF_ASSERT(slot < MAX_BOSSES);
  return _bosses[slot];
}

} // namespace okuu::stage

#define OKUU_SOL_IMPL
#include "../lua/sol.hpp"

#include "../lua/package.hpp"
#include "./stage.hpp"

#include "../render/instance.hpp"
#include "../render/stage.hpp"
#include <ntfstl/logger.hpp>

namespace okuu {

static constexpr u32 MAX_ENTITIES = render::stage_renderer::DEFAULT_STAGE_INSTANCES;

game_state::game_state(std::unique_ptr<assets::asset_bundle>&& assets,
                       std::unique_ptr<stage::stage_scene>&& scene, lua::stage_env&& lua_env) :
    _assets{std::move(assets)},
    _scene{std::move(scene)}, _lua_env{std::move(lua_env)}, _t{0.f} {}

expect<game_state> game_state::load_from_package(const std::string& path, chima::context& chima) {
  sol::state cfg_state;
  auto cfg = lua::package_cfg::load_config(cfg_state, path);
  if (!cfg.has_value()) {
    return {ntf::unexpect, std::move(cfg.error())};
  }

  auto renderer = okuu::render::stage_renderer::create(MAX_ENTITIES);
  if (!renderer.has_value()) {
    return {ntf::unexpect, std::move(renderer.error())};
  }

  if (cfg->players.size() == 0) {
    return {ntf::unexpect, "No players defined"};
  }

  if (cfg->stages.size() == 0) {
    return {ntf::unexpect, "No stages defined"};
  }

  auto& stage = cfg->stages[0];
  auto& player = cfg->players[0];
  const auto make_player = [&](const assets::sprite_atlas& atlas) -> stage::player_entity {
    stage::player_entity::animation_data player_anims;
    NTF_ASSERT(player_anims.size() == player.anim.size());
    for (u32 i = 0; const auto& [name, modifier] : player.anim) {
      auto atlas_anim = atlas.find_animation(name).value();
      player_anims[i] = {atlas_anim, modifier};
      ++i;
    }

    assets::sprite_animator player_animator{atlas, player_anims[0].first};
    const vec2 initial_pos{0.f, 0.f};
    return {initial_pos, std::move(player_anims), std::move(player_animator)};
  };

  auto assets = std::make_unique<assets::asset_bundle>();
  const auto put_asset = [&](const std::string& name, const lua::package_cfg::asset_elem& asset) {
    switch (asset.type) {
      case assets::asset_type::sprite_atlas: {
        chima::spritesheet sheet{chima, asset.path.c_str()};
        auto atlas = assets::sprite_atlas::from_chima(sheet).value();
        assets->emplace_asset<assets::sprite_atlas>(name, std::move(atlas));
      } break;
      default:
        NTF_UNREACHABLE();
    }
  };

  try {
    for (const auto& [name, asset] : cfg->assets) {
      logger::info("Loading asset \"{}\"", name);
      put_asset(name, asset);
    }
    logger::info("Loading player \"{}\"", player.name);
    const auto atlas_handle = assets->find_asset<assets::sprite_atlas>(player.sheet).value();
    const auto& player_atlas = assets->get_asset(atlas_handle);

    auto scene =
      std::make_unique<stage::stage_scene>(make_player(player_atlas), std::move(*renderer));
    auto lua_env = lua::stage_env::load(stage.script.c_str(), *scene).value();

    return {ntf::in_place, std::move(assets), std::move(scene), std::move(lua_env)};
  } catch (const std::exception& ex) {
    return {ntf::unexpect, ex.what()};
  }
}

void game_state::tick() {
  auto task_wait_ticks = _scene->task_wait();
  if (task_wait_ticks == 0) {
    _lua_env.run_tasks();
  } else {
    _scene->task_wait(task_wait_ticks - 1);
  }
  _scene->tick();
}

void game_state::render(f64 dt, f64 alpha) {
  _t += static_cast<f32>(dt);
  okuu::render::render_back(_t);
  _scene->render(dt, alpha);
}

namespace stage {

stage_scene::stage_scene(player_entity&& player, render::stage_renderer&& renderer) :
    _renderer{std::move(renderer)}, _projs{}, _bosses{}, _boss_count{}, _player{std::move(player)},
    _task_wait_ticks{0u}, _ticks{0u} {}

void stage_scene::render(double dt, double alpha) {
  // The scene has to render the following (in order):
  // - The background
  // - The boss(es)
  // - The player
  // - The items
  // - The danmaku
  NTF_UNUSED(dt);
  NTF_UNUSED(alpha);

  const auto render_sprite = [&](auto& entity) {
    const auto [tex, uvs] = entity.sprite();
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

} // namespace stage

} // namespace okuu

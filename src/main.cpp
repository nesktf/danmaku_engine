#include "./lua/stage_env.hpp"

#include "./lua/package.hpp"

#include <ntfstl/utility.hpp>

namespace okuu {

static constexpr u32 MAX_ENTITIES = render::stage_renderer::DEFAULT_STAGE_INSTANCES;

class game_state {
public:
  game_state(std::unique_ptr<assets::asset_bundle>&& assets,
             std::unique_ptr<stage::stage_scene>&& scene, lua::stage_env&& lua_env);

public:
  static expect<game_state> load_from_package(const std::string& path, chima::context& chima);

public:
  void tick();
  void render(f64 dt, f64 alpha);

private:
  std::unique_ptr<assets::asset_bundle> _assets;
  std::unique_ptr<stage::stage_scene> _scene;
  lua::stage_env _lua_env;
  f32 _t;
};

game_state::game_state(std::unique_ptr<assets::asset_bundle>&& assets,
                       std::unique_ptr<stage::stage_scene>&& scene, lua::stage_env&& lua_env) :
    _assets{std::move(assets)}, _scene{std::move(scene)}, _lua_env{std::move(lua_env)}, _t{0.f} {}

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
    auto lua_env = lua::stage_env::load(stage.script.c_str(), *scene, *assets).value();

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
  _scene->render(dt, alpha, *_assets);
}

static fn engine_run() {
  auto _rh = okuu::render::init();

  chima::context chima;
  auto state = okuu::game_state::load_from_package("res/packages/test/config.lua", chima);
  if (!state.has_value()) {
    okuu::logger::error("Failed to load stage: {}", state.error());
    return;
  }

  auto loop = ntf::overload{
    [&](double dt, double alpha) { state->render(dt, alpha); },
    [&](u32) { state->tick(); },
  };
  shogle::render_loop(okuu::render::window(), okuu::render::shogle_ctx(), okuu::GAME_UPS, loop);
}

} // namespace okuu

int main() {
  ntf::logger::set_level(ntf::log_level::verbose);
  try {
    okuu::engine_run();
  } catch (std::exception& ex) {
    ntf::logger::error("Caught {}", ex.what());
  } catch (...) {
    ntf::logger::error("Caught (...)");
  }
}

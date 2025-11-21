#define OKUU_SOL_IMPL
#include "../lua/sol.hpp"

#include "./package.hpp"

namespace okuu::lua {

namespace {

static constexpr f32 DEF_VEL_FAC = 1.f;
static constexpr f32 DEF_ACC_FAC = 1.f;
static constexpr f32 DEF_FOCUS_FAC = .8f;
static constexpr f32 DEF_HITBOX_FAC = 4.f;

fn make_setup_stages(std::vector<package_cfg::stage_entry>& stages, const std::string& dir) {
  return [&](sol::this_state, sol::table args) {
    try {
      sol::table tbl = args.as<sol::table>();
      tbl.for_each([&](sol::object, sol::object stage) {
        sol::table stage_tbl = stage.as<sol::table>();

        std::string path = fmt::format("{}/{}", dir, stage_tbl.get<std::string>("path"));
        std::string name = stage_tbl.get<std::string>("name");
        stages.emplace_back(std::move(name), std::move(path));
      });
    } catch (const sol::error& err) {
      logger::error("Malformed stage setup on lua script: {}", err.what());
    }
  };
}

fn make_setup_assets(package_cfg::asset_registry& registry, const std::string& dir) {
  return [&](sol::this_state, sol::table args) {
    try {
      args.for_each([&](sol::object key, sol::object value) {
        std::string name = key.as<std::string>();
        auto args = value.as<sol::table>();

        std::string path = fmt::format("{}/{}", dir, args.get<std::string>("path"));
        assets::asset_type type = static_cast<assets::asset_type>(args.get<u32>("type"));
        registry.try_emplace(std::move(name), std::move(path), type);
      });
    } catch (const sol::error& err) {
      logger::error("Malformed asset setup on lua script: {}", err.what());
    }
  };
}

fn make_setup_players(std::vector<package_cfg::player_userdata>& userdatas) {
  return [&](sol::this_state, sol::table args) {
    try {
      args.for_each([&](sol::object key, sol::object value) {
        std::string name = key.as<std::string>();
        auto args = value.as<sol::table>();

        std::string desc = args.get<std::string>("desc");
        std::string anim_sheet = args.get<std::string>("anim_sheet");
        auto stats = args.get<sol::table>("stats");
        f32 vel = stats.get_or("vel", DEF_VEL_FAC);
        f32 acc = stats.get_or("acc", DEF_ACC_FAC);
        f32 focus = stats.get_or("focus", DEF_FOCUS_FAC);
        f32 hitbox = stats.get_or("hitbox", DEF_HITBOX_FAC);

        decltype(package_cfg::player_userdata::anim) anims;
        auto anim_tbl = args.get<sol::table>("anim");
        u32 i = 0;
        anim_tbl.for_each([&](sol::object, sol::object anim_value) {
          auto tbl = anim_value.as<sol::table>();

          std::string name = tbl[1].get<std::string>();
          const bool invert = tbl[2].get<bool>();

          anims[i].first = std::move(name);
          if (invert) {
            anims[i].second = assets::sprite_animator::ANIM_BACKWARDS;
          } else {
            anims[i].second = assets::sprite_animator::ANIM_NO_MODIFIER;
          }
          ++i;
        });
        userdatas.emplace_back(std::move(name), std::move(desc), std::move(anim_sheet),
                               std::move(anims), vel, acc, focus, hitbox);
      });
    } catch (const sol::error& err) {
      logger::debug("Malformed player entry in lua script: {}", err.what());
    }
  };
}

} // namespace

package_cfg::package_cfg(asset_registry&& assets_, std::vector<player_userdata>&& players_,
                         std::vector<stage_entry>&& stages_) :
    assets{std::move(assets_)}, players{std::move(players_)}, stages{std::move(stages_)} {}

expect<package_cfg> package_cfg::load_config(sol::state_view lua, std::string script) {
  lua.open_libraries(sol::lib::base, sol::lib::coroutine, sol::lib::package, sol::lib::table,
                     sol::lib::math, sol::lib::string);
  asset_registry assets;
  std::vector<player_userdata> players;
  std::vector<stage_entry> stages;
  auto dir = shogle::file_dir(script).value();

  auto lib = lua["okuu"].get_or_create<sol::table>();
  auto package_module = lib["package"].get_or_create<sol::table>();
  package_module["register_stages"].set_function(make_setup_stages(stages, dir));
  package_module["register_assets"].set_function(make_setup_assets(assets, dir));
  package_module["register_players"].set_function(make_setup_players(players));

  auto assets_module = lib["assets"].get_or_create<sol::table>();
  auto aenums = assets_module["type"].get_or_create<sol::table>();
  aenums["sprite_atlas"] = static_cast<u32>(assets::asset_type::sprite_atlas);

  try {
    lua.safe_script_file(script);
    if (stages.empty()) {
      return {ntf::unexpect, "No stages loaded in script"};
    }
    return {ntf::in_place, std::move(assets), std::move(players), std::move(stages)};
  } catch (const sol::error& err) {
    return {ntf::unexpect, err.what()};
  }
}

} // namespace okuu::lua

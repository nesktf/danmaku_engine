#define OKUU_SOL_IMPL
#include "../lua/sol.hpp"

#include "./package.hpp"

namespace okuu::lua {

namespace {

static constexpr f32 DEF_VEL_FAC = 1.f;
static constexpr f32 DEF_ACC_FAC = 1.f;
static constexpr f32 DEF_FOCUS_FAC = .8f;
static constexpr f32 DEF_HITBOX_FAC = 4.f;

fn make_setup_stages(std::vector<package_cfg::stage_entry>& stages) {
  return [&](sol::this_state, sol::table args) {
    sol::table tbl = args.as<sol::table>();
    tbl.for_each([&](sol::object, sol::object stage) {
      sol::table stage_tbl = stage.as<sol::table>();

      try {
        stdfs::path path = stage_tbl.get<stdfs::path>("path");
        std::string name = stage_tbl.get<std::string>("name");
        stages.emplace_back(std::move(name), std::move(path));
      } catch (const sol::error& err) {
        logger::error("Malformed stage setup on lua script: {}", err.what());
      }
    });
  };
}

fn make_setup_assets(package_cfg::asset_registry& registry) {
  return [&](sol::this_state, sol::table args) {
    args.for_each([&](sol::object key, sol::object value) {
      std::string name = key.as<std::string>();
      auto args = value.as<sol::table>();

      try {
        stdfs::path path = args.get<stdfs::path>("path");
        assets::asset_type type = static_cast<assets::asset_type>(args.get<u32>("type"));
        registry.try_emplace(std::move(name), std::move(path), type);
      } catch (const sol::error& err) {
        logger::error("Malformed asset setup \"{}\" on lua script: {}", name, err.what());
      }
    });
  };
}

fn make_setup_players(std::vector<package_cfg::player_userdata>& userdatas) {
  return [&](sol::this_state, sol::table args) {
    args.for_each([&](sol::object key, sol::object value) {
      std::string name = key.as<std::string>();
      auto args = value.as<sol::table>();

      try {
        std::string desc = args.get<std::string>("desc");
        std::string anim_sheet = args.get<std::string>("anim_sheet");
        auto stats = args.get<sol::table>("stats");
        f32 vel = stats.get_or("vel", DEF_VEL_FAC);
        f32 acc = stats.get_or("acc", DEF_ACC_FAC);
        f32 focus = stats.get_or("focus", DEF_FOCUS_FAC);
        f32 hitbox = stats.get_or("hitbox", DEF_HITBOX_FAC);

        decltype(package_cfg::player_userdata::anim) anims;
        auto anim_tbl = stats.get<sol::table>("anim");
        u32 i = 0;
        anim_tbl.for_each([&](sol::object, sol::object anim_value) {
          auto tbl = anim_value.as<sol::table>();

          std::string name = tbl.get<std::string>(1);
          const bool invert = tbl.get<bool>(2);

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
      } catch (const sol::error& err) {
        logger::debug("Malformed player entry \"{}\" in lua script: {}", name, err.what());
      }
    });
  };
}

} // namespace

package_cfg::package_cfg(asset_registry&& assets_, std::vector<player_userdata>&& players_,
                         std::vector<stage_entry>&& stages_) :
    assets{std::move(assets_)}, players{std::move(players_)}, stages{std::move(stages_)} {}

expect<package_cfg> package_cfg::load_config(sol::state_view lua, std::string script) {
  asset_registry assets;
  std::vector<player_userdata> players;
  std::vector<stage_entry> stages;

  auto lib = lua["okuu"].get_or_create<sol::table>();
  auto package_module = lib["package"].get_or_create<sol::table>();
  package_module["register_stages"].set_function(make_setup_stages(stages));
  package_module["register_assets"].set_function(make_setup_assets(assets));
  package_module["register_players"].set_function(make_setup_players(players));

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

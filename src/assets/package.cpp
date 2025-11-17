#define OKUU_SOL_IMPL
#include "../lua/sol.hpp"

#include "./package.hpp"

namespace okuu::assets {

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
      stdfs::path path = stage_tbl.get<stdfs::path>("path");
      std::string name = stage_tbl.get<std::string>("name");
      stages.emplace_back(std::move(name), std::move(path));
    });
  };
}

fn make_setup_assets(assets::package_bundle& bundle) {
  return [&](sol::this_state, sol::table args) {
    args.for_each([&](sol::object key, sol::object value) {
      std::string name = key.as<std::string>();
      auto args = value.as<sol::table>();

      stdfs::path path = args.get<stdfs::path>("path");
      assets::asset_type type = static_cast<assets::asset_type>(args.get<u32>("type"));
      bundle.add_element(std::move(name), std::move(path), type, false);
    });
  };
}

fn make_setup_players(std::vector<package_cfg::player_userdata>& userdatas) {
  return [&](sol::this_state, sol::table args) {
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
    });
  };
}

} // namespace

package_bundle::package_bundle(chima::context& chima_) : chima{chima_} {}

package_bundle::~package_bundle() noexcept {
  for (auto& [_, elem] : registry) {
    if (elem.static_data == nullptr) {
      continue;
    }
    switch (elem.type) {
      case asset_type::spritesheet: {
        delete static_cast<chima::spritesheet*>(elem.static_data);
      } break;
      default:
        NTF_UNREACHABLE();
    }
  }
}

void package_bundle::add_element(std::string name, stdfs::path path, asset_type type,
                                 bool is_static) {
  void* static_data = nullptr;
  if (is_static) {
    switch (type) {
      case asset_type::spritesheet: {
        static_data = static_cast<void*>(new chima::spritesheet{*chima, path.c_str()});
      } break;
      default:
        NTF_UNREACHABLE();
    }
  }
  registry.try_emplace(std::move(name), std::move(path), type, static_data);
}

package_cfg::package_cfg(std::unique_ptr<assets::package_bundle>&& bundle_,
                         std::vector<player_userdata>&& players_,
                         std::vector<stage_entry>&& stages_) :
    bundle{std::move(bundle_)},
    players{std::move(players_)}, stages{std::move(stages_)} {}

expect<package_cfg> package_cfg::load_config(sol::state_view lua, std::string script,
                                             chima::context& chima) {
  auto bundle = std::make_unique<package_bundle>(chima);
  std::vector<player_userdata> players;
  std::vector<stage_entry> stages;

  auto lib = lua["okuu"].get_or_create<sol::table>();
  auto package_module = lib["package"].get_or_create<sol::table>();
  package_module["register_stages"].set_function(make_setup_stages(stages));
  package_module["register_assets"].set_function(make_setup_assets(*bundle));
  package_module["register_players"].set_function(make_setup_players(players));

  try {
    lua.safe_script_file(script);
    if (stages.empty()) {
      return {ntf::unexpect, "No stages loaded in script"};
    }
    return {ntf::in_place, std::move(bundle), std::move(players), std::move(stages)};
  } catch (const sol::error& err) {
    return {ntf::unexpect, err.what()};
  }
}

} // namespace okuu::assets

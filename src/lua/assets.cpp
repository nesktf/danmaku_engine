#define OKUU_SOL_IMPL
#include "./sol.hpp"

#include "./assets.hpp"

namespace okuu::lua {

lua_sprite::lua_sprite(assets::atlas_handle handle, assets::sprite_atlas::sprite sprite) :
    _handle{handle}, _sprite{sprite} {}

fn lua_sprite::get() const -> std::pair<assets::atlas_handle, assets::sprite_atlas::sprite> {
  return {_handle, _sprite};
}

lua_sprite_atlas::lua_sprite_atlas(assets::atlas_handle atlas) : _atlas{atlas} {}

sol::variadic_results lua_sprite_atlas::get_sprite(sol::this_state ts, std::string name) const {
  auto& bundle = lua_assets::instance(ts);
  sol::variadic_results ret;

  auto& atlas = bundle.get_asset(_atlas);
  auto sprite = atlas.find_sprite(name);
  if (!sprite.has_value()) {
    ret.push_back({ts, sol::nil});
    ret.push_back({ts, sol::in_place_type<std::string>, "Sprite not found"});
  }

  ret.push_back({ts, sol::in_place_type<lua_sprite>, _atlas, *sprite});
  return ret;
}

namespace {

fn prep_usertypes(sol::table& module) {
  // clang-format off
  module.new_usertype<lua_sprite>(
    "sprite", sol::no_constructor
  );
  module.new_usertype<lua_sprite_atlas>(
    "spritesheet", sol::no_constructor,
    "get_sprite", &lua_sprite_atlas::get_sprite
  );
  // clang-format on
}

fn prep_asset_funcs(sol::table& module) {
  module.set_function("require", [](sol::this_state ts, std::string name) {
    logger::debug("Requiring asset \"{}\"", name);

    auto& bundle = lua_assets::instance(ts);
    sol::variadic_results ret;

    auto atlas_handle = bundle.find_asset<assets::sprite_atlas>(name);
    if (!atlas_handle.has_value()) {
      ret.push_back({ts, sol::nil});
      ret.push_back({ts, sol::in_place_type<std::string>, "Asset not found"});
      return ret;
    }

    ret.push_back({ts, sol::in_place_type<lua_sprite_atlas>, *atlas_handle});
    return ret;
  });
}

} // namespace

lua_assets::lua_assets(assets::asset_bundle& assets) : _assets{assets} {}

sol::table lua_assets::setup_module(sol::table& okuu_lib, assets::asset_bundle& assets) {
  sol::table asset_module = okuu_lib["assets"].get_or_create<sol::table>();
  okuu_lib["__curr_assets"] = lua_assets{assets};
  prep_usertypes(asset_module);
  prep_asset_funcs(asset_module);
  return asset_module;
}

assets::asset_bundle& lua_assets::instance(sol::state_view lua) {
  lua_assets module = lua["okuu"]["__curr_assets"].get<lua_assets>();
  return module.get();
}

} // namespace okuu::lua

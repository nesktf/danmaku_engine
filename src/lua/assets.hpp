#pragma once

#include "./sol.hpp"

#include "../assets/manager.hpp"

namespace okuu::lua {

class lua_sprite {
public:
  lua_sprite(assets::atlas_handle handle, assets::sprite_atlas::sprite sprite);

public:
  std::pair<assets::atlas_handle, assets::sprite_atlas::sprite> get() const;

private:
  assets::atlas_handle _handle;
  assets::sprite_atlas::sprite _sprite;
};

class lua_sprite_atlas {
public:
  lua_sprite_atlas(assets::atlas_handle atlas);

public:
  sol::variadic_results get_sprite(sol::this_state ts, std::string name) const;

private:
  assets::atlas_handle _atlas;
};

class lua_assets {
public:
  lua_assets(assets::asset_bundle& assets);

public:
  assets::asset_bundle& get() { return *_assets; }

public:
  static sol::table setup_module(sol::table& okuu_lib, assets::asset_bundle& assets);
  static assets::asset_bundle& instance(sol::state_view lua);

private:
  ntf::weak_ptr<assets::asset_bundle> _assets;
};

} // namespace okuu::lua

#pragma once

#include "../assets/manager.hpp"
#include "./sol.hpp"

namespace okuu::lua {

namespace stdfs = std::filesystem;

class package_cfg {
public:
  struct stage_entry {
    std::string name;
    stdfs::path script;
  };

  enum player_anim_entry {
    PLAYER_IDLE = 0,
    PLAYER_LEFT,
    PLAYER_RIGHT,
    PLAYER_IDLE_LEFT,
    PLAYER_LEFT_IDLE,
    PLAYER_IDLE_RIGHT,
    PLAYER_RIGHT_IDLE,

    PLAYER_ANIM_COUNT,
  };

  using chara_sprites =
    std::array<std::pair<std::string, assets::sprite_animator::anim_modifier>, PLAYER_ANIM_COUNT>;

  struct player_userdata {
    std::string name;
    std::string desc;
    std::string sheet;
    chara_sprites anim;
    f32 vel;
    f32 acc;
    f32 focus;
    f32 hitbox;
  };

  struct asset_elem {
    stdfs::path path;
    assets::asset_type type;
  };

  using asset_registry = std::unordered_map<std::string, asset_elem>;

public:
  package_cfg(asset_registry&& assets_, std::vector<player_userdata>&& players_,
              std::vector<stage_entry>&& stages_);

public:
  static expect<package_cfg> load_config(sol::state_view lua, std::string script);

public:
  asset_registry assets;
  std::vector<player_userdata> players;
  std::vector<stage_entry> stages;
};

} // namespace okuu::lua

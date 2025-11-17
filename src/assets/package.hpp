#pragma once

#include "../lua/sol.hpp"
#include "./sprite.hpp"

namespace okuu::assets {

namespace stdfs = std::filesystem;

enum class asset_type {
  spritesheet = 0,
};

template<typename T>
struct asset_type_mapper {};

template<>
struct asset_type_mapper<chima::spritesheet> :
    public std::integral_constant<asset_type, asset_type::spritesheet> {};

template<typename T>
concept is_asset_type =
  std::same_as<asset_type, std::remove_cv_t<decltype(asset_type_mapper<T>::value)>>;

class package_bundle {
public:
  struct registry_elem {
    template<is_asset_type T>
    T* data_as() const {
      return static_cast<T*>(static_data);
    }

    stdfs::path path;
    asset_type type;
    void* static_data;
  };

public:
  package_bundle(chima::context& chima_);

  NTF_DECLARE_NO_MOVE_NO_COPY(package_bundle);

public:
  void add_element(std::string name, stdfs::path path, asset_type type, bool is_static);

public:
  ntf::weak_ptr<chima::context> chima;
  std::unordered_map<std::string, registry_elem> registry;
};

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

  struct player_userdata {
    std::string name;
    std::string desc;
    std::string sheet;
    std::array<std::pair<std::string, sprite_animator::anim_modifier>, PLAYER_ANIM_COUNT> anim;
    f32 vel;
    f32 acc;
    f32 focus;
    f32 hitbox;
  };

public:
  package_cfg(std::unique_ptr<assets::package_bundle>&& bundle_,
              std::vector<player_userdata>&& players_, std::vector<stage_entry>&& stages_);

public:
  static expect<package_cfg> load_config(sol::state_view lua, std::string script,
                                         chima::context& chima);

public:
  std::unique_ptr<package_bundle> bundle;
  std::vector<player_userdata> players;
  std::vector<stage_entry> stages;
};

} // namespace okuu::assets

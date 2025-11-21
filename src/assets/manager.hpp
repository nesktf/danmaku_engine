#pragma once

#include "./sprite.hpp"

namespace okuu::assets {

enum class asset_type {
  sprite_atlas = 0,
};

template<typename T>
struct asset_enum_mapper {};

template<>
struct asset_enum_mapper<sprite_atlas> :
    public std::integral_constant<asset_type, asset_type::sprite_atlas> {};

template<typename T>
constexpr asset_type asset_enum_mapper_v = asset_enum_mapper<T>::value;

template<asset_type>
struct asset_type_mapper {};

template<>
struct asset_type_mapper<asset_type::sprite_atlas> {
  using type = sprite_atlas;
};

template<asset_type type>
using asset_type_mapper_t = asset_type_mapper<type>::type;

template<typename T>
concept manager_asset = requires() {
  requires std::same_as<asset_type, std::remove_cv_t<decltype(asset_enum_mapper<T>::value)>>;
};

template<asset_type type>
class asset_handle {
public:
  explicit asset_handle(u32 idx) noexcept : _idx{idx} {}

public:
  u32 get() const { return _idx; }

private:
  u32 _idx;
};

using atlas_handle = assets::asset_handle<assets::asset_type::sprite_atlas>;

class asset_bundle {
public:
  asset_bundle() = default;

public:
  template<manager_asset T>
  fn find_asset(const std::string& name) -> ntf::optional<asset_handle<asset_enum_mapper_v<T>>> {
    static constexpr auto type = asset_enum_mapper_v<T>;
    const auto make_handle = [](u32 handle) -> asset_handle<type> {
      return asset_handle<type>{handle};
    };

    if constexpr (type == asset_type::sprite_atlas) {
      auto it = _atlas_map.find(name);
      if (it == _atlas_map.end()) {
        return {ntf::nullopt};
      }
      return {ntf::in_place, make_handle(it->second)};
    }
  }

  template<manager_asset T, typename... Args>
  fn emplace_asset(std::string name, Args&&... args) -> asset_handle<asset_enum_mapper_v<T>> {
    static constexpr auto type = asset_enum_mapper_v<T>;
    u32 idx;
    if constexpr (type == asset_type::sprite_atlas) {
      idx = static_cast<u32>(_atlas_vec.size());
      _atlas_vec.emplace_back(std::forward<Args>(args)...);
      _atlas_map.emplace(std::move(name), idx);
    }
    return asset_handle<type>{idx};
  };

  template<asset_type type>
  fn get_asset(asset_handle<type> handle) -> asset_type_mapper_t<type>& {
    const u32 idx = handle.get();
    if constexpr (type == asset_type::sprite_atlas) {
      NTF_ASSERT(idx < _atlas_vec.size());
      return _atlas_vec[idx];
    }
  }

private:
  std::unordered_map<std::string, u32> _atlas_map;
  std::vector<sprite_atlas> _atlas_vec;
};

} // namespace okuu::assets

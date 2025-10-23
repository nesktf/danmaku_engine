#pragma once

#include "./chima.hpp"

#include <ntfstl/expected.hpp>
#include <ntfstl/unique_array.hpp>

#include <queue>
#include <shogle/shogle.hpp>

namespace okuu::assets {

using namespace ntf::numdefs;

template<typename T>
using expect = ntf::expected<T, std::string>;

class sprite_atlas {
private:
  struct sprite_uvs {
    f32 x_lin, x_con;
    f32 y_lin, y_con;
  };

  struct anim_meta {
    f32 fps;
    u32 start_idx;
    u32 count;
  };

public:
  enum class sprite : u32 {};
  enum class animation : u32 {};

public:
  sprite_atlas(shogle::texture2d&& tex, ntf::unique_array<sprite_uvs>&& uvs,
               std::unordered_map<std::string, u32>&& sprite_map,
               ntf::unique_array<anim_meta>&& anim_pos,
               std::unordered_map<std::string, u32>&& anim_map);

public:
  expect<sprite_atlas> from_chima(const chima::spritesheet& sheet);

public:
  ntf::optional<sprite> find_sprite(std::string_view name) const;
  ntf::optional<animation> find_animation(std::string_view name) const;

  std::pair<shogle::texture2d_view, sprite_uvs> render_data(sprite spr) const;

  u32 anim_length(animation anim) const;
  sprite anim_sprite_at(animation anim, u32 tick, u32 ups) const;

private:
  shogle::texture2d _tex;
  ntf::unique_array<sprite_uvs> _sprite_uvs;
  std::unordered_map<std::string, u32> _sprite_map;
  ntf::unique_array<anim_meta> _anim_pos;
  std::unordered_map<std::string, u32> _anim_map;
};

class sprite_animator {
private:
  struct anim_entry {
    sprite_atlas::animation anim;
    u32 duration;
    u32 timer;
  };

public:
  sprite_animator(const sprite_atlas& atlas, sprite_atlas::animation first_anim);

public:
  void enqueue(sprite_atlas::animation anim, u32 loops);
  void enqueue_frames(sprite_atlas::animation anim, u32 frames);
  void soft_switch(sprite_atlas::animation anim, u32 loops);
  void hard_switch(sprite_atlas::animation anim, u32 loops);

public:
  void tick();
  sprite_atlas::sprite frame() const;

  const sprite_atlas& atlas() const { return *_atlas; }

private:
  void _reset_queue(bool hard);

private:
  ntf::weak_cptr<sprite_atlas> _atlas;
  std::queue<anim_entry> _anim_queue;
};

} // namespace okuu::assets

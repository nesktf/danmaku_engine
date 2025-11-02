#pragma once

#include "../core.hpp"
#include "../render/stage.hpp"
#include "./chima.hpp"

#include <ntfstl/unique_array.hpp>

#include <queue>

namespace okuu::assets {

class sprite_atlas {
private:
  struct anim_meta {
    u32 fps;
    u32 start_idx;
    u32 count;
  };

public:
  enum class sprite : u32 {};
  enum class animation : u32 {};

public:
  sprite_atlas(shogle::texture2d&& tex, ntf::unique_array<render::sprite_uvs>&& uvs,
               std::unordered_map<std::string, u32>&& sprite_map,
               ntf::unique_array<anim_meta>&& anim_pos,
               std::unordered_map<std::string, u32>&& anim_map);

public:
  static expect<sprite_atlas> from_chima(const chima::spritesheet& sheet);

public:
  ntf::optional<sprite> find_sprite(std::string_view name) const;
  ntf::optional<animation> find_animation(std::string_view name) const;

  std::pair<shogle::texture2d_view, render::sprite_uvs> render_data(sprite spr) const;

  u32 anim_length(animation anim) const;
  sprite anim_sprite_at(animation anim, u32 tick) const;

private:
  shogle::texture2d _tex;
  ntf::unique_array<render::sprite_uvs> _sprite_uvs;
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

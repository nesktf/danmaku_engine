#include "./sprite.hpp"
#include "../render/common.hpp"

namespace okuu::assets {

sprite_atlas::sprite_atlas(shogle::texture2d&& tex, ntf::unique_array<render::sprite_uvs>&& uvs,
                           std::unordered_map<std::string, u32>&& sprite_map,
                           ntf::unique_array<anim_meta>&& anim_pos,
                           std::unordered_map<std::string, u32>&& anim_map) :
    _tex{std::move(tex)},
    _sprite_uvs{std::move(uvs)}, _sprite_map{std::move(sprite_map)},
    _anim_pos{std::move(anim_pos)}, _anim_map{std::move(anim_map)} {}

expect<sprite_atlas> sprite_atlas::from_chima(const chima::spritesheet& sheet) {
  const auto parse_sheet = [&](shogle::texture2d&& atlas_tex) -> sprite_atlas {
    const auto sprites = sheet.sprites();
    ntf::unique_array<render::sprite_uvs> uvs(sprites.size());
    std::unordered_map<std::string, u32> sprite_map;
    for (u32 i = 0; const auto& sprite : sprites) {
      uvs[i].x_lin = sprite.uv_x_lin;
      uvs[i].x_con = sprite.uv_x_con;
      uvs[i].y_lin = sprite.uv_y_lin;
      uvs[i].y_con = sprite.uv_y_con;

      std::string name{sprite.name.data, sprite.name.length};
      [[maybe_unused]] auto [it, empl] = sprite_map.try_emplace(std::move(name), i);
      NTF_ASSERT(empl);
      ++i;
    }

    const auto anims = sheet.anims();
    ntf::unique_array<anim_meta> anim_pos(anims.size());
    std::unordered_map<std::string, u32> anim_map;
    for (u32 i = 0; const auto& anim : anims) {
      anim_pos[i].start_idx = anim.sprite_idx;
      anim_pos[i].count = anim.sprite_count;
      anim_pos[i].fps = static_cast<u32>(std::round(anim.fps));

      std::string name{anim.name.data, anim.name.length};
      [[maybe_unused]] auto [it, empl] = anim_map.try_emplace(std::move(name), i);
      NTF_ASSERT(empl);
      ++i;
      ++i;
    }

    return {std::move(atlas_tex), std::move(uvs), std::move(sprite_map), std::move(anim_pos),
            std::move(anim_map)};
  };

  const auto [width, height] = sheet.atlas_extent();
  return render::create_texture(width, height, sheet.atlas_data()).transform(parse_sheet);
}

auto sprite_atlas::find_sprite(std::string_view name) const -> ntf::optional<sprite> {
  auto it = _sprite_map.find({name.data(), name.size()});
  if (it == _sprite_map.end()) {
    return {ntf::nullopt};
  }
  return {ntf::in_place, static_cast<sprite>(it->second)};
}

auto sprite_atlas::find_animation(std::string_view name) const -> ntf::optional<animation> {
  auto it = _anim_map.find({name.data(), name.size()});
  if (it == _anim_map.end()) {
    return {ntf::nullopt};
  }
  return {ntf::in_place, static_cast<animation>(it->second)};
}

auto sprite_atlas::render_data(sprite spr) const
  -> std::pair<shogle::texture2d_view, render::sprite_uvs> {
  const u32 idx = static_cast<u32>(spr);
  NTF_ASSERT(idx < _sprite_uvs.size());
  return {_tex, _sprite_uvs[idx]};
}

u32 sprite_atlas::anim_length(animation anim) const {
  const u32 anim_idx = static_cast<u32>(anim);
  NTF_ASSERT(anim_idx < _anim_pos.size());
  return _anim_pos[anim_idx].count;
}

auto sprite_atlas::anim_sprite_at(animation anim, u32 tick) const -> sprite {
  const u32 anim_idx = static_cast<u32>(anim);
  NTF_ASSERT(anim_idx < _anim_pos.size());
  const auto& anim_data = _anim_pos[anim_idx];
  const u32 frame_idx = anim_data.start_idx + (tick % anim_length(anim));
  NTF_ASSERT(frame_idx < _sprite_uvs.size());
  return static_cast<sprite>(frame_idx);
}

sprite_animator::sprite_animator(const sprite_atlas& atlas, sprite_atlas::animation first_anim) :
    _atlas{atlas}, _anim_queue{} {
  enqueue(first_anim, 0);
}

void sprite_animator::enqueue(sprite_atlas::animation anim, u32 loops) {
  const anim_entry entry{
    .anim = anim,
    .duration = loops * _atlas->anim_length(anim),
    .timer = 0,
  };
  _anim_queue.push(entry);
}

void sprite_animator::enqueue_frames(sprite_atlas::animation anim, u32 frames) {
  enqueue(anim, 0);
  _anim_queue.back().duration = frames;
}

void sprite_animator::soft_switch(sprite_atlas::animation anim, u32 loops) {
  _reset_queue(false);
  enqueue(anim, loops);
}

void sprite_animator::hard_switch(sprite_atlas::animation anim, u32 loops) {
  _reset_queue(true);
  enqueue(anim, loops);
}

void sprite_animator::_reset_queue(bool hard) {
  if (_anim_queue.size() == 0) {
    return;
  }

  auto front = std::move(_anim_queue.front());
  while (!_anim_queue.empty()) {
    _anim_queue.pop();
  }
  if (!hard) {
    _anim_queue.push(std::move(front));
  }
}

void sprite_animator::tick() {
  auto& next = _anim_queue.front();
  const u32 anim_len = _atlas->anim_length(next.anim);
  auto& timer = next.timer;
  ++timer;
  if (timer >= next.duration && _anim_queue.size() > 1 && timer % anim_len == 0) {
    _anim_queue.pop();
  }
}

sprite_atlas::sprite sprite_animator::frame() const {
  const auto& next = _anim_queue.front();
  return _atlas->anim_sprite_at(next.anim, next.timer);
}

} // namespace okuu::assets

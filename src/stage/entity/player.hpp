#pragma once

#include "global.hpp"
#include "resources.hpp"

#include "render.hpp"

#include <shogle/scene/transform.hpp>

namespace entity {

class player {
public:

  enum anim_state : uint8_t {
    IDLE = 0,
    LEFT,
    LEFT_TO_IDLE,
    IDLE_TO_LEFT,
    RIGHT,
    RIGHT_TO_IDLE,
    IDLE_TO_RIGHT,
    ANIM_COUNT,
  };

  using anim_data = std::array<res::atlas_type::sequence_handle, ANIM_COUNT>;

public:
  player() = default;

public:
  void tick();

public:
  void set_sprite(res::atlas atlas, anim_data anim) {
    _anim = anim;
    _animator = res::sprite_animator{atlas, anim[0]};

    const auto& seq = atlas->sequence_at(anim[0]);
    const auto& meta = atlas->at(seq[0]);
    transf.set_scale(meta.aspect()*scale);
  }

  template<typename Vec>
  void set_pos(Vec vec) {
    transf.set_pos(vec);
  }

  void set_scale(float sc) {
    scale = sc;
  }

  res::sprite sprite() const {
    return res::sprite {
      .handle = _animator.atlas(),
      .index = _animator.frame(),
    };
  }

  ntf::transform2d& transform() { return transf; }
  uint birth() const { return _birth; }

  const renderer::uniform_tuple& uniforms() const { return _uniforms; }

public:
  ntf::transform2d transf;
  renderer::uniform_tuple _uniforms;

  float scale {40.0f};
  float speed_factor{450.0f};
  bool shifting{false};
  uint _birth{0};

  res::sprite_animator _animator;
  anim_data _anim;
  anim_state _state{IDLE};
};

} // namespace entity

#pragma once

#include "../render/common.hpp"

namespace okuu::stage {

class entity_animator {
public:
  void tick(frames entity_ticks);

public:
  res::sprite sprite() const;

public:
  res::atlas handle;
  res::atlas_type::texture_handle index{};
  res::atlas_type::sequence_handle sequence{};
  bool use_sequence{false};
};

} // namespace okuu::stage

#pragma once

#include <shogle/shogle.hpp>

#include <shogle/res/spritesheet.hpp>
#include <shogle/res/model.hpp>

#include <shogle/res/shader/sprite.hpp>
#include <shogle/res/shader/model.hpp>

#include <shogle/res/meshes.hpp>

namespace ntf {

struct entity2d;

class sprite_renderer {
public:
  sprite_renderer();

public:
  void operator()(const shogle::camera2d& cam, const entity2d& obj);

private:
  shogle::mesh _quad{};
  shogle::sprite_shader _shader{};
};

class model_renderer {
public:
  model_renderer() = default;

public:
  void operator()(const shogle::camera3d& cam, const shogle::transform3d& transform, shogle::model& model);

private:
  shogle::model_shader _shader{};
};

} // namespace ntf

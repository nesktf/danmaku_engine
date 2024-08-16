#pragma once

#include "core.hpp"

#include <shogle/render/shader.hpp>

#include <shogle/res/pool.hpp>
#include <shogle/res/atlas.hpp>
#include <shogle/res/font.hpp>

namespace res {

void init();

class shader {
public:
  shader() = default;

  shader(ntf::resource_id id) : _shader(id) {}

  shader(std::string_view name);

public:
  ntf::resource_id id() const { return _shader; }
  const ntf::shader_program& get() const;

  bool valid() const;
  operator bool() const { return valid(); }

private:
  ntf::resource_id _shader{};
};


class sprite;
class sprite_atlas {
public:
  sprite_atlas() = default;

  sprite_atlas(ntf::resource_id id) : _atlas(id) {}

  sprite_atlas(std::string_view name);

public:
  ntf::resource_id id() const { return _atlas; }
  const ntf::texture_atlas& get() const;
  sprite at(ntf::texture_atlas::texture tex) const;

  bool valid() const;
  operator bool() const { return valid(); }

public:
  static sprite_atlas default_atlas();

private:
  ntf::resource_id _atlas{};
};


class sprite {
public:
  sprite() = default;

  sprite(sprite_atlas atlas, ntf::texture_atlas::texture tex) :
    _atlas(atlas), _tex(tex) {}

  sprite(ntf::resource_id atlas, ntf::texture_atlas::texture tex) :
    _atlas(atlas), _tex(tex) {}

  sprite(std::string_view atlas, ntf::texture_atlas::texture tex) :
    _atlas(atlas), _tex(tex) {}

public:
  const ntf::texture_atlas::texture_meta& get_meta() const;
  const ntf::texture2d& get_tex() const;

  bool valid() const;
  operator bool() const { return valid(); }

private:
  sprite_atlas _atlas;
  ntf::texture_atlas::texture _tex{};
};


class font {
public:
  font() = default;

  font(ntf::resource_id id) : _font(id) {}

  font(std::string_view name);

public:
  ntf::resource_id id() const { return _font; }
  const ntf::font& get() const;

  bool valid() const;
  operator bool() const { return valid(); }

private:
  ntf::resource_id _font;
};

} // namespace res

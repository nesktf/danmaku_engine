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

  shader(ntf::resource_id id) :
    _shader(id) {}

  shader(std::string_view name);

public:
  ntf::resource_id id() const { return _shader; }
  const ntf::shader_program& get() const;

  bool valid() const;
  operator bool() const { return valid(); }
  operator const ntf::shader_program&() const { return get(); }

private:
  ntf::resource_id _shader{};
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
  operator const ntf::font&() const { return get(); }

private:
  ntf::resource_id _font;
};


class sprite;
class sprite_atlas {
public:
  sprite_atlas() = default;

  sprite_atlas(ntf::resource_id id) :
    _atlas(id) {}

  sprite_atlas(std::string_view name);

public:
  ntf::resource_id id() const { return _atlas; }
  const ntf::texture_atlas& get() const;

  sprite at(ntf::texture_atlas::texture tex) const;
  bool valid() const;
  operator bool() const { return valid(); }
  operator const ntf::texture_atlas&() const { return get(); }

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
  ntf::resource_id id() const { return _tex; }

  const ntf::texture_atlas::texture_meta& meta() const;
  const sprite_atlas& atlas() const { return _atlas; }
  const ntf::texture2d& tex() const;

  bool valid() const;
  operator bool() const { return valid(); }
  operator const ntf::texture2d&() const { return tex(); }

private:
  sprite_atlas _atlas;
  ntf::texture_atlas::texture _tex{};
};

} // namespace res

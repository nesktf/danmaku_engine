#pragma once

#include <shogle/res/spritesheet.hpp>
#include <shogle/res/font.hpp>

namespace ntf {

template<typename T>
using strmap = std::unordered_map<std::string, T>;


class animated_sprite {
public:
  animated_sprite(const shogle::sprite* sprite, uint delay, std::initializer_list<uint> indices);

public:
  void tick();

public:
  uint index() { return _indices[_index]; }
  const shogle::sprite& sprite() const { return *_sprite; }

private:
  const shogle::sprite* _sprite;
  uint _delay, _time{0}, _index{0};
  std::vector<uint> _indices;
};



struct resources_t {
  strmap<shogle::spritesheet> sprites;
  strmap<shogle::font> fonts;
};

extern resources_t& res;

void resources_init();

} // namespace ntf

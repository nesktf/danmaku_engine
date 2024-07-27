#include "resources.hpp"



namespace ntf {

static resources_t _res;

resources_t& res = _res;

auto filter = shogle::tex_filter::nearest;
auto wrap   = shogle::tex_wrap::repeat;

void resources_init() {
  auto& sprites = _res.sprites;
  sprites.emplace(std::make_pair("enemies", 
    shogle::load_spritesheet("res/spritesheets/enemies.json", filter, wrap)));
  sprites.emplace(std::make_pair("effects", 
    shogle::load_spritesheet("res/spritesheets/effects.json", filter, wrap)));
  sprites.emplace(std::make_pair("chara", 
    shogle::load_spritesheet("res/spritesheets/chara.json", filter, wrap)));
}


animated_sprite::animated_sprite(const shogle::sprite* sprite, uint delay, std::initializer_list<uint> indices) :
  _sprite(sprite), _delay(delay), _indices(indices) {}

void animated_sprite::tick() {
  if (_time++ >= _delay) {
    _index = (_index + 1) % _indices.size();
    _time = 0;
  }
}

} // namespace ntf

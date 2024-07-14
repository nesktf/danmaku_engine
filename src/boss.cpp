#include "boss.hpp"
#include "render.hpp"

namespace ntf::game {

boss::boss(shogle::sprite* sprite, cmplx pos) : _sprite(sprite) {
  _transf.set_pos(pos)
    .set_scale(68.0f*sprite->corrected_scale());
}

void boss::render() {
  render_sprite(*_sprite, _transf, 0);
}

}

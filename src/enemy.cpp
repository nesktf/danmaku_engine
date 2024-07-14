#include "enemy.hpp"
#include "render.hpp"
#include "state.hpp"

#include <shogle/core/log.hpp>
#include <shogle/math/collision.hpp>

namespace ntf::game {

enemy::enemy(shogle::sprite* sprite, velf velfun, cmplx pos, cmplx dir) :
  _sprite(sprite), _velfun(velfun), _dir(dir) {
    _transf.set_pos(pos)
      .set_scale(50.0f);
}

void enemy::render() {
  render_sprite(*_sprite, _transf, 0);
}

}

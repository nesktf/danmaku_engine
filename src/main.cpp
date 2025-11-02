#include "stage/stage.hpp"

#include <ntfstl/logger.hpp>
#include <ntfstl/utility.hpp>

using namespace ntf::numdefs;

void engine_run() {
  auto _rh = okuu::render::init();

  chima::context chima;
  chima::spritesheet sheet{chima, "res/spritesheet/chara.chima"};
  auto atlas = okuu::assets::sprite_atlas::from_chima(sheet).value();
  auto stage = okuu::stage::lua_env::load("res/packages/test/main.lua", atlas).value();

  float t = 0.f;
  auto loop = ntf::overload{
    [&](double dt, double alpha) {
      t += (float)dt;
      okuu::render::render_back(t);
      stage.scene().render(dt, alpha);
    },
    [&](u32) { stage.tick(); },
  };
  shogle::render_loop(okuu::render::window(), okuu::render::shogle_ctx(), 60, loop);
}

int main() {
  ntf::logger::set_level(ntf::log_level::verbose);
  try {
    engine_run();
  } catch (std::exception& ex) {
    ntf::logger::error("Caught {}", ex.what());
  } catch (...) {
    ntf::logger::error("Caught (...)");
  }
}

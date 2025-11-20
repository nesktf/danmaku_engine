#include "./stage/stage.hpp"

#include <ntfstl/logger.hpp>
#include <ntfstl/utility.hpp>

using namespace ntf::numdefs;

static void engine_run() {
  auto _rh = okuu::render::init();

  chima::context chima;
  auto state = okuu::game_state::load_from_package("res/packages/test/config.lua", chima);
  if (!state.has_value()) {
    okuu::logger::error("Failed to load stage: {}", state.error());
    return;
  }

  auto loop = ntf::overload{
    [&](double dt, double alpha) { state->render(dt, alpha); },
    [&](u32) { state->tick(); },
  };
  shogle::render_loop(okuu::render::window(), okuu::render::shogle_ctx(), okuu::GAME_UPS, loop);
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

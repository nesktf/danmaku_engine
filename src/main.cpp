#include "stage/stage.hpp"

#include <ntfstl/logger.hpp>
#include <ntfstl/utility.hpp>

using namespace ntf::numdefs;

static okuu::expect<okuu::stage::lua_env> load_stage(chima::context& chima) {
  chima::spritesheet chara_sheet{chima, "res/packages/test/chara.chima"};
  for (auto& anim : chara_sheet.anims()) {
    auto name = chima::to_str_view(anim.name);
    if (name == "chara_cirno.idle") {
      anim.fps = 20.f;
    } else if (name.find("chara_") != std::string::npos) {
      anim.fps = 30.f;
    }
  }

  std::vector<okuu::assets::sprite_atlas> resources;
  auto& chara_atlas =
    resources.emplace_back(okuu::assets::sprite_atlas::from_chima(chara_sheet).value());

  auto cirno_idle = chara_atlas.find_animation("chara_cirno.idle").value();
  auto cirno_right = chara_atlas.find_animation("chara_cirno.right").value();
  auto cirno_idle_right = chara_atlas.find_animation("chara_cirno.idle_to_right").value();
  auto cirno_left = chara_atlas.find_animation("chara_cirno.left").value();
  auto cirno_idle_left = chara_atlas.find_animation("chara_cirno.idle_to_left").value();

  okuu::assets::sprite_animator chara_animator{chara_atlas, cirno_idle};

  okuu::stage::player_entity::animation_data anims;
  anims[okuu::stage::player_entity::IDLE] = {cirno_idle,
                                             okuu::assets::sprite_animator::ANIM_NO_MODIFIER};
  anims[okuu::stage::player_entity::RIGHT] = {cirno_right,
                                              okuu::assets::sprite_animator::ANIM_NO_MODIFIER};
  anims[okuu::stage::player_entity::RIGHT_TO_IDLE] = {
    cirno_idle_right, okuu::assets::sprite_animator::ANIM_BACKWARDS};
  anims[okuu::stage::player_entity::IDLE_TO_RIGHT] = {
    cirno_idle_right,
    okuu::assets::sprite_animator::ANIM_NO_MODIFIER,
  };
  anims[okuu::stage::player_entity::LEFT] = {cirno_left,
                                             okuu::assets::sprite_animator::ANIM_NO_MODIFIER};
  anims[okuu::stage::player_entity::LEFT_TO_IDLE] = {
    cirno_idle_left, okuu::assets::sprite_animator::ANIM_BACKWARDS};
  anims[okuu::stage::player_entity::IDLE_TO_LEFT] = {
    cirno_idle_left, okuu::assets::sprite_animator::ANIM_NO_MODIFIER};

  okuu::stage::player_entity player{okuu::vec2{0.f, 0.f}, std::move(anims),
                                    std::move(chara_animator)};

  static constexpr u32 max_entities = okuu::render::stage_renderer::DEFAULT_STAGE_INSTANCES;
  auto scene = std::make_unique<okuu::stage::stage_scene>(max_entities, std::move(player),
                                                          std::move(resources));

  return okuu::stage::lua_env::load("res/packages/test/main.lua", std::move(scene));
}

static void engine_run() {
  auto _rh = okuu::render::init();

  chima::context chima;
  auto stage = load_stage(chima).value();

  float t = 0.f;
  auto loop = ntf::overload{
    [&](double dt, double alpha) {
      t += (float)dt;
      okuu::render::render_back(t);
      stage.scene().render(dt, alpha);
    },
    [&](u32) { stage.tick(); },
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

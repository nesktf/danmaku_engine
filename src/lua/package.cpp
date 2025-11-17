#include "./lua_env.hpp"
#include <string>

namespace okuu::stage {

fn make_start_stage(std::vector<lua_package_cfg::stage_funcs>& funcs) {
  return [&](sol::this_state, sol::table args) {
    auto setup = args.get<sol::protected_function>("setup");
    auto run = args.get<sol::protected_function>("run");
    if (!setup.valid() || !run.valid()) {
      return;
    }
    funcs.emplace_back(std::move(setup), std::move(run));
  };
};

sol::table lua_env::init_package(sol::state_view lua, assets::package_bundle& bundle,
                                 std::vector<player_userdata>& userdatas,
                                 std::vector<stage_entry>& stages,
                                 std::vector<stage_funcs>& stage_funcs) {
  sol::table module = lua.create_table();
  module["register_stages"].set_function(make_setup_stages(stages));
  module["register_assets"].set_function(make_setup_assets(bundle));
  module["register_players"].set_function(make_setup_players(userdatas));
  module["start_stage"].set_function(make_start_stage(stage_funcs));
  return module;
}

} // namespace okuu::stage

#include "global.hpp"
#include "frontend.hpp"
#include "package.hpp"

#include <shogle/engine.hpp>

static void init_stage(frontend::menu_entry& entry) {
  global::start_stage(entry.text);
}

static void close_game(frontend::menu_entry&) {
  ntf::engine_close_window();
}

static frontend::menu _build_package_view() {
  ntf::transform2d t;
  t.set_pos((vec2)VIEWPORT*0.5f).set_scale(50.0f);

  frontend::menu menu {
    .entries = {},
    .on_tick = [](frontend::menu& m){
      auto& t = m.back_transform;
      t.set_rot(t.rot() + PI*DT);
    },
    .background = {2, 3},
    .back_transform = std::move(t),
  };
  auto packages = package::parse_packages();
  if (packages.empty()) {
    menu.entries = {
      {.text = "go back", .on_click = [](auto&){frontend::instance().pop();}}
    };
  } else {
    for (const auto& package : packages) {
      menu.entries.emplace_back(package, init_stage);
    }
    menu.entries.emplace_back("go back", [](auto&){frontend::instance().pop();});
  }

  return menu;
}

static frontend::menu _build_settings() {
  ntf::transform2d t;
  t.set_pos((vec2)VIEWPORT*0.5f).set_scale(50.0f);
  return frontend::menu {
    .entries = {
      {.text = "nothing1", .on_click = [](auto&){}},
      {.text = "nothing2", .on_click = [](auto&){}},
      {.text = "nothing3", .on_click = [](auto&){}},
      {.text = "nothing4", .on_click = [](auto&){}},
      {.text = "go back", .on_click = [](auto&){frontend::instance().pop();}},
    },
    .on_tick = [](frontend::menu& m){
      auto& t = m.back_transform;
      t.set_rot(t.rot() + PI*DT);
    },
    .background = {3, 3},
    .back_transform = std::move(t),
  };
}

static frontend::menu _build_main_menu() {
  ntf::transform2d t;
  t.set_pos((vec2)VIEWPORT*0.5f).set_scale(50.0f);
  return frontend::menu {
    .entries = {
      {.text = "start", .on_click = [](auto&){frontend::instance().push(_build_package_view());}},
      {.text = "settings", .on_click = [](auto&){frontend::instance().push(_build_settings());}},
      {.text = "exit", .on_click = close_game},
    },
    .on_tick = [](frontend::menu& m){
      auto& t = m.back_transform;
      t.set_rot(t.rot() + PI*DT);
    },
    .background = {3, 2},
    .back_transform = std::move(t),
  };
}


void frontend::state_init() {
  _entries.push(_build_main_menu());
}


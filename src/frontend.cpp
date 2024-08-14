#include "global.hpp"
#include "frontend.hpp"
#include "shogle/engine.hpp"

static frontend::frontend _state;

namespace frontend {
static void nothing() {}
static void pop() { _state.pop(); }

static void load_new() {
  _state.push(menu{vector<menu_entry> {
    menu_entry{.text = "nothing1", .on_click = nothing},
    menu_entry{.text = "nothing2", .on_click = nothing},
    menu_entry{.text = "nothing3", .on_click = nothing},
    menu_entry{.text = "nothing4", .on_click = nothing},
    menu_entry{.text = "go back", .on_click = pop}
  }, res::sprite_id{.index = 1, .sheet = 2}});
  _state.entry().back_transform.set_pos((vec2)VIEWPORT*.5f).set_scale(50.0f);
}

void frontend::init() {
  // const auto sheet_id = res::spritesheet_index("default");
  _entries.emplace(vector<menu_entry>{
    menu_entry{.text = "start", .on_click = global::start_stage},
    menu_entry{.text = "settings", .on_click = load_new},
    menu_entry{.text = "exit", .on_click = ntf::engine_close_window}
  }, res::sprite_id{.index = 1, .sheet = 3});
  _entries.top().back_transform.set_pos((vec2)VIEWPORT*.5f).set_scale(50.0f);
}

void frontend::tick() {
  static float t = 0.0f;
  t += DT;
  auto& curr = entry();
  curr.back_transform.set_rot(PI*t);
}

} // namespace frontend

void frontend::init() {
  _state.init();
}

void frontend::tick() {
  _state.tick();
}

frontend::frontend& frontend::state() {
  return _state;
}

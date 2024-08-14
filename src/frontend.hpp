#pragma once

#include "core.hpp"
#include "resources.hpp"

#include <shogle/scene/transform.hpp>

#include <stack>

namespace frontend {

using menu_index = uint16_t;

struct menu_entry {
  std::string text;
  void (*on_click)();
};

class menu {
public:
  menu(std::vector<menu_entry> entries_, res::sprite_id back, menu_index index = 0): 
    focused(index), entries(std::move(entries_)), background(back) {}

public:
  void set_next_index() { focused = (focused+1) % entries.size(); }
  void set_prev_index() { focused = focused == 0 ? entries.size()-1 : focused-1;}
  void on_click() { (entries[focused].on_click)(); }

public:
  menu_index focused{0};
  std::vector<menu_entry> entries;
  res::sprite_id background;
  ntf::transform2d back_transform;
};

class frontend {
public:
  void init();
  void tick();

public:
  menu& entry() { return _entries.top(); }
  void pop() { 
    if (_entries.size() > 1) {
      _entries.pop();
    }
  }
  void push(menu menu) { _entries.push(std::move(menu)); }

private:
  std::stack<menu> _entries;
};

void init();
void tick();
frontend& state();

} // namespace frontend

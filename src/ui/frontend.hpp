#pragma once

#include "global.hpp"
#include "resources.hpp"

#include <shogle/scene/transform.hpp>

#include <shogle/core/singleton.hpp>

#include <stack>

class frontend : public ntf::singleton<frontend> {
public:
  using menu_index = uint16_t;

  struct menu_entry {
    std::string text;
    void (*on_click)(menu_entry&);
  };

  class menu {
  public:
    void set_next_index() { focused = (focused+1) % entries.size(); }
    void set_prev_index() { focused = focused == 0 ? entries.size()-1 : focused-1;}
    void on_click() { 
      auto& entry = entries[focused];
      (*entry.on_click)(entry); 
    }
    void tick() { (*on_tick)(*this); }

  public:
    std::vector<menu_entry> entries;
    void (*on_tick)(menu&);
    res::sprite background;
    ntf::transform2d back_transform{};
    menu_index focused{0};
  };

public:
  void pop() { 
    if (_entries.size() > 1) {
      _entries.pop();
    }
  }
  void push(menu menu) { _entries.push(std::move(menu)); }

public:
  menu& entry() { return _entries.top(); }

public:
  static void init() { instance().state_init(); }
  static void tick() { instance().entry().tick(); }

private:
  void state_init();

private:
  std::stack<menu> _entries;
};

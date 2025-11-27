#pragma once

#include <functional>
#include <list>
#include <string>
#include <unordered_map>

namespace okuu::util {

template<typename F>
class event_handler {
public:
  using event_iterator = std::list<F>::iterator;

public:
  event_handler() = default;

public:
  template<typename Func>
  event_iterator register_event(Func&& func) {
    return _events.emplace(_events.end(), std::forward<Func>(func));
  }

  template<typename... Args>
  void trigger_event(Args&&... args) {
    for (auto& event : _events) {
      std::invoke(event, std::forward<Args>(args)...);
    }
  }

  void unregister_event(event_iterator event) { _events.erase(event); }

  void clear_events() { _events.clear(); }

private:
  std::list<F> _events;
};

template<typename F>
class multi_event_handler {
public:
  using event_iterator = event_handler<F>::event_iterator;

public:
  multi_event_handler() = default;

public:
  template<typename Func>
  event_iterator register_event(const std::string& name, Func&& func) {
    auto it = _events.find(name);
    if (it == _events.end()) {
      auto [it2, empl] = _events.try_emplace(std::move(name));
      it = it2;
    }
    NTF_ASSERT(it != _events.end());
    return it->second.register_event(std::forward<Func>(func));
  }

  template<typename... Args>
  void trigger_event(const std::string& name, Args&&... args) {
    auto it = _events.find(name);
    if (it == _events.end()) {
      return;
    }
    it->second.trigger_event(std::forward<Args>(args)...);
  }

  void unregister_event(const std::string& name, event_iterator event) {
    auto it = _events.find(name);
    if (it == _events.end()) {
      return;
    }
    it->second.unregister_event(event);
  }

  void clear_events(const std::string& name) {
    auto it = _events.find(name);
    if (it == _events.end()) {
      return;
    }
    it->second.clear_events();
  }

  void clear_events() {
    for (auto& [_, handler] : _events) {
      handler.clear_events();
    }
  }

private:
  std::unordered_map<std::string, event_handler<F>> _events;
};

} // namespace okuu::util

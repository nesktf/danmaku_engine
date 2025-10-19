#pragma once

#include <functional>
#include <list>

namespace okuu {

template<typename... Args>
class event_handler {
private:
  using fun_t = std::function<void(Args...)>;

public:
  using event = std::list<fun_t>::const_iterator;

public:
  event_handler() = default;

public:
  template<typename F>
  event suscribe(F&& callback) {
    return _events.emplace(_events.end(), std::forward<F>(callback));
  }

  void unsuscribe(event e) {
    _events.erase(e);
  }

public:
  void fire(Args... args) {
    for (auto& event : _events) {
      std::invoke(event, std::forward<Args>(args)...);
    }
  }

  void clear() {
    _events.clear();
  }
  
private:
  std::list<fun_t> _events;
};

} // namespace okuu

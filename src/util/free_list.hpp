#pragma once

#include <ntfstl/optional.hpp>
#include <ntfstl/ptr.hpp>
#include <ntfstl/span.hpp>
#include <ntfstl/types.hpp>

#include <queue>

namespace okuu::util {

using namespace ntf::numdefs;

template<typename T>
class free_list {
private:
  struct storage {
    ntf::nullable<T> obj;
    u32 version;
  };

public:
  class element {
  public:
    element(free_list& list, u64 handle) : _list{list}, _handle{handle} {}

  public:
    T& operator*() {
      NTF_ASSERT(_list._is_valid(_handle));
      return _list._elem_at(_handle);
    }

    const T& operator*() const {
      NTF_ASSERT(_list._is_valid(_handle));
      return _list._elem_at(_handle);
    }

    T* operator->() { return &**this; }

    const T* operator->() const { return &**this; }

    T& get() { return **this; }

    const T& get() const { return **this; }

    bool is_alive() const { return _list._is_valid(_handle); }

    void kill() { _list->return_elem(_handle); }

    u64 handle() const { return _handle; }

  private:
    ntf::weak_ptr<free_list> _list;
    u64 _handle;

    friend class free_list;
  };

  friend class element;

public:
  free_list() : _elems{}, _free{} {}

public:
  template<typename... Args>
  element request_elem(Args&&... args) {
    if (!_free.empty()) {
      const u32 idx = _free.back();
      _free.pop();
      auto& [elem, version] = _elems[idx];
      NTF_ASSERT(!elem.has_value());
      elem.emplace(std::forward<Args>(args)...);
      ++version;
      return {*this, _compose_handle(idx, version)};
    }

    const u32 idx = _elems.size() - 1u;
    auto& [elem, version] = _elems.emplace_back(ntf::nullopt, 0);
    elem.emplace(std::forward<Args>(args)...);
    return {*this, _compose_handle(idx, version)};
  }

  free_list& return_elem(u64 handle) {
    const auto [idx, _] = _decompose_handle(handle);
    NTF_ASSERT(idx < _elems.size());
    auto& [elem, version] = _elems[idx];
    NTF_ASSERT(elem.has_value());
    elem.reset();
    ++version;
    _free.push(idx);
    return *this;
  }

  void clear() {
    for (auto& [elem, version] : _elems) {
      if (elem.has_value()) {
        elem.reset();
        ++version;
      }
    }
    while (!_free.empty()) {
      _free.pop();
    }
  }

  template<typename F>
  free_list& clear_where(F&& func) {
    for (u32 idx = 0; auto& [elem, version] : _elems) {
      if (!elem.has_value()) {
        ++idx;
        continue;
      }
      const bool should_reset = std::invoke(func, *elem);
      if (!should_reset) {
        ++idx;
        continue;
      }

      elem.reset();
      ++version;
      _free.push(idx);
      ++idx;
    }
  }

  template<typename F>
  free_list& for_each(F&& func) {
    for (auto& [elem, _] : _elems) {
      if (elem.has_value()) {
        std::invoke(func, *elem);
      }
    }
    return *this;
  }

  template<typename F>
  const free_list& for_each(F&& func) const {
    for (auto& [elem, _] : _elems) {
      if (elem.has_value()) {
        std::invoke(func, *elem);
      }
    }
    return *this;
  }

private:
  bool _is_valid(u64 handle) const {
    const auto [idx, ver] = _decompose_handle(handle);
    if (idx > _elems.size()) {
      return false;
    }
    const auto& elem = _elems[idx];
    if (!elem.has_value() || elem.version != ver) {
      return false;
    }

    return true;
  }

  static u64 _compose_handle(u32 idx, u32 ver) { return (ver << 32) | idx; }

  static std::pair<u32, u32> _decompose_handle(u64 handle) {
    return {handle & 0xFFFFFFFF, handle >> 32};
  }

private:
  std::vector<storage> _elems;
  std::queue<u32> _free;
};

} // namespace okuu::util

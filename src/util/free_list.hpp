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
public:
  class element {
  public:
    element(free_list& list, u32 idx) : _list{list}, _idx{idx} {}

  public:
    T& operator*() {
      NTF_ASSERT(_idx < _list->_elems.size());
      auto& proj = _list->_elems[_idx];
      NTF_ASSERT(proj.has_value());
      return *proj;
    }

    const T& operator*() const {
      NTF_ASSERT(_idx < _list->_elems.size());
      auto& proj = _list->_elems[_idx];
      NTF_ASSERT(proj.has_value());
      return *proj;
    }

    bool alive() const {
      NTF_ASSERT(_idx < _list->_elems.size());
      auto& proj = _list->_elems[_idx];
      return proj.has_value();
    }

    T* operator->() { return &**this; }

    const T* operator->() const { return &**this; }

    u32 idx() const { return _idx; }

  private:
    ntf::weak_ptr<free_list> _list;
    u32 _idx;

    friend class free_list;
  };

  friend class element;

public:
  free_list() : _elems{}, _free{} {}

public:
  template<typename... Args>
  element request_elem(Args&&... args) {
    if (!_free.empty()) {
      const u32 pos = _free.back();
      _free.pop();
      auto& elem = _elems[pos];
      elem.emplace(std::forward<Args>(args)...);
      return {*this, pos};
    }

    _elems.emplace(std::in_place, std::forward<Args>(args)...);
    return {*this, static_cast<u32>(_elems.size()) - 1u};
  }

  void return_elem(element elem) {
    const u32 idx = elem._idx;
    NTF_ASSERT(idx < _elems.size());
    auto& proj = _elems[idx];
    NTF_ASSERT(proj.has_value());
    proj.reset();
    _free.push(idx);
  }

  void clear() {
    _elems.clear();
    while (!_free.empty()) {
      _free.pop();
    }
  }

  // ntf::cspan<ntf::nullable<T>> elems() const { return {_elems.data(), _elems.size()}; }

private:
  std::vector<ntf::nullable<T>> _elems;
  std::queue<u32> _free;
};

} // namespace okuu::util

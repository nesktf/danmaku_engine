#pragma once

#include "global.hpp"

namespace script {

class package {
public:
  package(std::string path) :
    _path(std::move(path)) {}

public:
  std::string_view path() const { return _path; }

private:
  std::string _path;

public:
  static std::vector<package> parse_packages(std::string_view package_root = "");

public:
  static const std::string_view default_root;
};

} // namespace script

#pragma once

#include "global.hpp"

#include <filesystem>

namespace package {

namespace fs = std::filesystem;

class package {
public:
  package(fs::path path);

public:
  std::string_view path() const { return _path.c_str(); }

private:
  fs::path _path;
};

std::vector<package> parse(std::string_view package_root = "");

} // namespace package

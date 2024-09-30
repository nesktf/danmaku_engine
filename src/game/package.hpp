#pragma once

#include "global.hpp"
#include "resources.hpp"

namespace game {

class context;

class package {
public:
  package(std::string path);

public:
  std::unique_ptr<context> make_context();

private:
  std::string _path;
};

std::vector<package> parse_packages(std::string_view package_root = "");

} // namespace game

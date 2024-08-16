#include "package.hpp"

#include <filesystem>

static const char* pack_path = "res/packages/";

static bool check_package(std::string& path, const std::filesystem::path& root) {
  const auto main = root / "main.lua";

  if (std::filesystem::exists(main)) {
    path = main.string();
    return true;
  }

  return false;
}

namespace package {

std::vector<std::string> parse_packages() {
  std::vector<std::string> packs;

  for (const auto& entry : std::filesystem::directory_iterator(pack_path)) {
    if (!entry.is_directory()) {
      continue;
    }

    std::string path;
    if (check_package(path, entry.path())) {
      packs.emplace_back(std::move(path));
    }
  }

  return packs;
}

} // namespace package

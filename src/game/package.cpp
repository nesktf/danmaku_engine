#include "game/context.hpp"
#include "game/package.hpp"

#include <shogle/core/log.hpp>

#include <filesystem>

namespace fs = std::filesystem;

namespace {

bool check_and_get_package(std::string& path, const std::filesystem::path& root) {
  const auto main = root / "main.lua";

  if (std::filesystem::exists(main)) {
    path = main.string();
    return true;
  }

  return false;
}

const std::string_view default_root = "res/packages";

} // namespace


namespace game {

package::package(std::string path) :
  _path(std::move(path)) {}

std::unique_ptr<context> package::make_context() {
  auto ctx = std::make_unique<context>(context::context_args{
    .script = _path,
    .viewport = VIEWPORT,
  });
  return ctx;
}

std::vector<package> parse_packages(std::string_view package_root) {
  std::vector<package> packs;

  fs::directory_iterator it;
  if (package_root.empty()) {
    ntf::log::warning("[script::package] Using default package root for parsing");
    it = fs::directory_iterator(default_root);
  } else {
    it = fs::directory_iterator(package_root);
  }

  for (const auto& entry : it) {
    if (!entry.is_directory()) {
      continue;
    }

    std::string path;
    if (check_and_get_package(path, entry.path())) {
      packs.emplace_back(std::move(path));
    }
  }

  ntf::log::debug("[script::package] Found {} packages", packs.size());
  return packs;
}

} // namespace game

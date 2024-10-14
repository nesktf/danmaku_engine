#include "package.hpp"

static std::string_view _default_root = "res/packages";

namespace package {

package::package(fs::path path) : _path(std::move(path)) {}

std::vector<package> parse(std::string_view package_root) {
  std::vector<package> packs;

  fs::directory_iterator it;
  if (package_root.empty()) {
    ntf::log::warning("[script::package] Using default package root for parsing");
    it = fs::directory_iterator{_default_root};
  } else {
    it = fs::directory_iterator{package_root};
  }

  for (const auto& entry : it) {
    if (!entry.is_directory()) {
      continue;
    }
    const auto main = entry.path() / "main.lua";

    if (!fs::exists(main)) {
      continue;
    }

    packs.emplace_back(main.string());
  }

  ntf::log::debug("[script::package] Found {} packages", packs.size());
  return packs;
}

} // namespace package

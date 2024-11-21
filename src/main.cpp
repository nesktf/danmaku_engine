#include "context.hpp"

int main([[maybe_unused]] const int argc, [[maybe_unused]] const char* argv[]) {
#ifdef NDEBUG
  okuu::logger::set_level(ntf::log_level::info);
#else
  okuu::logger::set_level(ntf::log_level::verbose);
#endif

  auto glfw = ntf::glfw_init();
  if (!glfw) {
    okuu::logger::fatal("[okuu::main] Failed to initialize GLFW: %s", glfw.error());
    return EXIT_FAILURE;
  }
  glfw.set_swap_interval(0);
  glfw.apply_hints(ntf::glfw_hints{
    .profile = ntf::glfw_profile::core,
    .context_ver_maj = 3,
    .context_ver_min = 3,
    .x11_class_name = "test",
    .x11_instance_name = "test",
  });

  std::string_view win_title = "okuu engine - shogle ver. " SHOGLE_VERSION_STRING;
  ntf::glfw_window<ntf::gl_context> window{1280, 720, win_title};
  if (!window) {
    okuu::logger::fatal("[okuu::main] Failed to create GLFW window: %s", glfw.error());
    return EXIT_FAILURE;
  }

  ntf::shogle_main_loop(window, okuu::UPS, okuu::context{window});

  return EXIT_SUCCESS;
}

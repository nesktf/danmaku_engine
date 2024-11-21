#include "context.hpp"

int main([[maybe_unused]] const int argc, [[maybe_unused]] const char* argv[]) {
#ifdef NDEBUG
  okuu::logger::set_level(ntf::log_level::info);
#else
  okuu::logger::set_level(ntf::log_level::verbose);
#endif

  auto glfw = ntf::glfw_init();
  if (!glfw) {
    okuu::logger::fatal("Failed to initialize GLFW!");
    return EXIT_FAILURE;
  }
  glfw.set_swap_interval(0);

  ntf::glfw_window<ntf::gl_context> window{1280, 720, "test", 3, 3};
  if (!window) {
    okuu::logger::fatal("Failed to create GLFW window!");
    return EXIT_FAILURE;
  }

  ntf::shogle_main_loop(window, okuu::UPS, okuu::context{window});

  return EXIT_SUCCESS;
}

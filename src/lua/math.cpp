#define OKUU_SOL_IMPL
#include "./sol.hpp"

#include "../core.hpp"

namespace okuu {

namespace {

fn add_logger(sol::table& lib) {
  auto logger = lib["logger"].get_or_create<sol::table>();
  logger.set_function("error", [](std::string msg) { ntf::logger::error("{}", msg); });
  logger.set_function("warn", [](std::string msg) { ntf::logger::warning("{}", msg); });
  logger.set_function("debug", [](std::string msg) { ntf::logger::debug("{}", msg); });
  logger.set_function("info", [](std::string msg) { ntf::logger::info("{}", msg); });
  logger.set_function("verbose", [](std::string msg) { ntf::logger::info("{}", msg); });
  return logger;
}

fn add_cmplx(sol::table& lib) {
  return lib.new_usertype<cmplx>(
    "cmplx", sol::call_constructor,
    sol::constructors<cmplx(), cmplx(float), cmplx(float, float)>{}, "real",
    sol::property(
      +[](cmplx& c, float f) { c.real(f); }, +[](cmplx& c) -> float { return c.real(); }),
    "imag",
    sol::property(
      +[](cmplx& c, float f) { c.imag(f); }, +[](cmplx& c) -> float { return c.imag(); }),
    "expi", &shogle::expic, "polar", &std::polar<float>,

    sol::meta_function::addition, sol::resolve<cmplx(const cmplx&, const cmplx&)>(&std::operator+),
    sol::meta_function::subtraction,
    sol::resolve<cmplx(const cmplx&, const cmplx&)>(&std::operator-),
    sol::meta_function::multiplication,
    sol::overload(sol::resolve<cmplx(const cmplx&, const cmplx&)>(&std::operator*),
                  sol::resolve<cmplx(const cmplx&, const float&)>(&std::operator*),
                  sol::resolve<cmplx(const float&, const cmplx&)>(&std::operator*)),
    sol::meta_function::division,
    sol::overload(sol::resolve<cmplx(const cmplx&, const cmplx&)>(&std::operator/),
                  sol::resolve<cmplx(const cmplx&, const float&)>(&std::operator/),
                  sol::resolve<cmplx(const float&, const cmplx&)>(&std::operator/)));
}

fn add_vec2(sol::table& lib) {
  // clang-format off
  return lib.new_usertype<vec2>(
    "vec2",
      sol::call_constructor, sol::constructors<vec2(), vec2(float), vec2(float, float)>{},
    "x",
      sol::property(+[](vec2& v, float f) { v.x = f; }, +[](vec2& v) -> float { return v.x; }),
    "y",
      sol::property(+[](vec2& v, float f) { v.y = f; }, +[](vec2& v) -> float { return v.y; }),
    sol::meta_function::addition,
      sol::resolve<vec2(const vec2&, const vec2&)>(&glm::operator+),
    sol::meta_function::subtraction,
      sol::resolve<vec2(const vec2&, const vec2&)>(&glm::operator-),
    sol::meta_function::multiplication,
      sol::resolve<vec2(const vec2&, const vec2&)>(&glm::operator*),
    sol::meta_function::division,
      sol::resolve<vec2(const vec2&, const vec2&)>(&glm::operator/)
  );
  // clang-format on
}

fn add_mat4(sol::table& lib) {
  // clang-format off
  return lib.new_usertype<mat4>(
    "mat4",
      sol::call_constructor, sol::constructors<mat4()>{},
    sol::meta_function::multiplication,
      sol::resolve<mat4(const mat4&, const mat4&)>(&glm::operator*)
  );
  // clang-format on
}

} // namespace

sol::table setup_math_usertypes(sol::state_view lua) {
  auto lib = lua["okuu"].get_or_create<sol::table>();
  add_logger(lib);

  auto math_module = lib["math"].get_or_create<sol::table>();
  add_cmplx(math_module);
  add_vec2(math_module);
  add_mat4(math_module);

  return lib;
}

} // namespace okuu

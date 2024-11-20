#include "package/lua_state.hpp"
#include "resources.hpp"

#include <shogle/scene/transform.hpp>

namespace {

void stlib_log(sol::table& module) {
  module.set_function("error", [](std::string msg) { logger::error("{}", msg); });
  module.set_function("warn", [](std::string msg) { logger::warning("{}", msg); });
  module.set_function("debug", [](std::string msg) { logger::debug("{}", msg); });
  module.set_function("info", [](std::string msg) { logger::info("{}", msg); });
  module.set_function("verbose", [](std::string msg) { logger::info("{}", msg); });
}

void stlib_math(sol::table& module) {
  auto cmplx_type = module.new_usertype<cmplx>(
    "cmplx", sol::call_constructor,
    sol::constructors<cmplx(), cmplx(real), cmplx(real, real)>{},
    "real", sol::property(
      +[](cmplx& c, real f) { c.real(f); },
      +[](cmplx& c) -> real { return c.real(); }
    ),
    "imag", sol::property(
      +[](cmplx& c, real f) { c.imag(f); },
      +[](cmplx& c) -> real { return c.imag(); }
    ),
    "expi", &ntf::expic,
    "polar", &std::polar<real>,

    sol::meta_function::addition,
      sol::resolve<cmplx(const cmplx&, const cmplx&)>(&std::operator+),
    sol::meta_function::subtraction,
      sol::resolve<cmplx(const cmplx&, const cmplx&)>(&std::operator-),
    sol::meta_function::multiplication, sol::overload(
      sol::resolve<cmplx(const cmplx&, const cmplx&)>(&std::operator*),
      sol::resolve<cmplx(const cmplx&, const real&)>(&std::operator*),
      sol::resolve<cmplx(const real&, const cmplx&)>(&std::operator*)
    ),
    sol::meta_function::division, sol::overload(
      sol::resolve<cmplx(const cmplx&, const cmplx&)>(&std::operator/),
      sol::resolve<cmplx(const cmplx&, const real&)>(&std::operator/),
      sol::resolve<cmplx(const real&, const cmplx&)>(&std::operator/)
    )
  );

  auto vec3_type = module.new_usertype<vec3>(
    "vec3", sol::call_constructor,
    sol::constructors<vec3(), vec3(real, real, real)>{},
    "x", &vec3::x,
    "y", &vec3::y,
    "z", &vec3::z,
    
    sol::meta_function::addition,
      sol::resolve<vec3(const vec3&, const vec3&)>(&glm::operator+),
    sol::meta_function::subtraction,
      sol::resolve<vec3(const vec3&, const vec3&)>(&glm::operator-),
    sol::meta_function::multiplication, sol::overload(
      sol::resolve<vec3(const vec3&, const vec3&)>(&glm::operator*),
      sol::resolve<vec3(real, const vec3&)>(&glm::operator*),
      sol::resolve<vec3(const vec3&, real)>(&glm::operator*)
    ),
    sol::meta_function::division, sol::overload(
      sol::resolve<vec3(const vec3&, const vec3&)>(&glm::operator/),
      sol::resolve<vec3(real, const vec3&)>(&glm::operator/),
      sol::resolve<vec3(const vec3&, real)>(&glm::operator/)
    )
  );

  auto transf2d_type = module.new_usertype<ntf::transform2d>(
    "transform2d", sol::call_constructor,
    sol::constructors<ntf::transform2d()>{},
    "pos", sol::property(
      +[](ntf::transform2d& t, cmplx pos) { t.pos(pos); },
      +[](ntf::transform2d&t ) -> cmplx { return t.cpos(); }
    ),
    "scale", sol::property(
      +[](ntf::transform2d& t, cmplx scale) { t.scale(scale); },
      +[](ntf::transform2d& t) -> cmplx { return t.cscale(); }
    ),
    "rot", sol::property(
      +[](ntf::transform2d& t, real rot) { t.rot(rot); },
      +[](ntf::transform2d& t) -> real { return t.rot(); }
    )
  );
}

void stlib_res(sol::table& module) {
  // auto sprite_type = module.new_usertype<res::sprite>(
  //   "sprite", sol::no_constructor
  // );
  // auto tex_type = module.new_usertype<res::texture>(
  //   "texture", sol::no_constructor
  // );
  // auto shader_type = module.new_usertype<res::shader>(
  //   "shader", sol::no_constructor
  // );
  // auto font_type = module.new_usertype<res::font>(
  //   "font", sol::no_constructor
  // );
}


using module_pair = std::pair<const std::string_view, void(*)(sol::table&)>;

constexpr module_pair modules[] {
  std::pair{"log", &stlib_log},
  std::pair{"math", &stlib_math},
  std::pair{"res", &stlib_res},
};
constexpr std::size_t module_count = sizeof(modules)/sizeof(module_pair);

constexpr std::string_view incl_path = ";res/script/?.lua";

} // namespace


sol::table package::stlib_open(sol::state_view lua) {
  lua.open_libraries(
    sol::lib::base, sol::lib::coroutine,
    sol::lib::package, sol::lib::table,
    sol::lib::math, sol::lib::string
  );

  lua["package"]["path"] = incl_path.data();

  auto stlib = lua[stlib_key.data()].get_or_create<sol::table>();
  for (std::size_t i = 0; i < module_count; ++i) {
    const auto& [name, loader] = modules[i];
    auto module = stlib[name.data()].get_or_create<sol::table>();
    loader(module);
  }

  return stlib;
}

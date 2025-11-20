#include "./stage_env.hpp"
#include "../stage/stage.hpp"

namespace okuu::lua {

namespace {

constexpr u32 secs_to_ticks(f32 secs) noexcept {
  return static_cast<u32>(std::floor(secs * okuu::GAME_UPS));
}

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

fn setup_base_module(sol::state_view lua) {
  auto okuu_lib = lua["okuu"].get_or_create<sol::table>();
  add_logger(okuu_lib);

  auto math_module = okuu_lib["math"].get_or_create<sol::table>();
  add_cmplx(math_module);
  add_vec2(math_module);
  add_mat4(math_module);

  return okuu_lib;
}

struct stage_data {
  sol::protected_function stage_setup;
  sol::protected_function stage_run;
};

fn setup_package_module(sol::table& okuu_lib, ntf::optional<stage_data>& stage_data) {
  auto package_module = okuu_lib["package"].get_or_create<sol::table>();

  package_module["start_stage"].set_function([&](sol::this_state, sol::table args) {
    auto setup = args.get<sol::optional<sol::protected_function>>("setup");
    auto run = args.get<sol::optional<sol::protected_function>>("run");
    if (!setup.has_value() || !run.has_value()) {
      return;
    }
    stage_data.emplace(std::move(*setup), std::move(*run));
  });

  return package_module;
}

fn clean_package_module(sol::table& okuu_lib) {
  okuu_lib["package"].set(sol::nil);
}

} // namespace

thread_coro::thread_coro(sol::thread&& coro_thread_, sol::coroutine&& coro_) :
    _coro_thread{std::move(coro_thread_)}, _coro{std::move(coro_)} {}

thread_coro thread_coro::from_func(sol::protected_function func) {
  auto state = func.lua_state();
  auto thread = sol::thread::create(state);
  sol::coroutine coro{thread.state(), func};
  return {std::move(thread), std::move(coro)};
}

sol::table setup_stage_module(sol::table& okuu_lib);
sol::table setup_assets_module(sol::table& okuu_lib);

stage_env::stage_env(sol::state&& lua, sol::thread&& stage_run_thread,
                     sol::coroutine&& stage_run) :
    _lua{std::move(lua)},
    _stage_run_thread{std::move(stage_run_thread)}, _stage_run{std::move(stage_run)} {}

static constexpr std::string_view incl_path = ";res/script/?.lua";

expect<stage_env> load(const std::string& script_path, stage::stage_scene& scene,
                       assets::asset_bundle& assets) {
  sol::state lua;
  lua.open_libraries(sol::lib::base, sol::lib::coroutine, sol::lib::package, sol::lib::table,
                     sol::lib::math, sol::lib::string);
  lua["package"]["path"] = incl_path.data();
  auto okuu_lib = setup_base_module(lua);
  auto stage_module = setup_stage_module(okuu_lib);
  auto asset_module = setup_assets_module(okuu_lib);

  ntf::optional<stage_data> stage_data;
  auto package_module = setup_package_module(okuu_lib, stage_data);

  try {
    lua.safe_script_file(script_path);
    clean_package_module(okuu_lib); // stage_data will become a dangling pointer otherwise
    if (!stage_data.has_value()) {
      return {ntf::unexpect, "No stage functions defined in lua scriptl"};
    }

    auto setup_res = std::invoke(stage_data->stage_setup);
    if (!setup_res.valid()) {
      sol::error err = setup_res;
      logger::error("Error on script stage_setup \"{}\": {}", script_path, err.what());
      return {ntf::unexpect, err.what()};
    }

    auto run_thread = sol::thread::create(lua.lua_state());
    sol::coroutine run_coro{run_thread.state(), stage_data->stage_run};
    okuu_lib["__curr_stage"] = lua_stage{scene};
    okuu_lib["__curr_asset"] = lua_asset_bundle{assets};

    return {ntf::in_place, std::move(lua), std::move(run_thread), std::move(run_coro)};
  } catch (const sol::error& err) {
    return {ntf::unexpect, err.what()};
  }
}

void stage_env::run_tasks() {
  auto okuu_lib = _lua["okuu"].get<sol::table>();
  auto stage = okuu_lib["__curr_stage"];
  std::invoke(_stage_run, stage);
}

} // namespace okuu::lua

#include "./stage_env.hpp"

#include "./assets.hpp"
#include "./stage.hpp"

namespace okuu::lua {

namespace {

fn setup_okuu_base(sol::table& okuu_lib) {
  auto logger = okuu_lib["logger"].get_or_create<sol::table>();
  logger.set_function("error", [](std::string msg) { ntf::logger::error("{}", msg); });
  logger.set_function("warn", [](std::string msg) { ntf::logger::warning("{}", msg); });
  logger.set_function("debug", [](std::string msg) { ntf::logger::debug("{}", msg); });
  logger.set_function("info", [](std::string msg) { ntf::logger::info("{}", msg); });
  logger.set_function("verbose", [](std::string msg) { ntf::logger::info("{}", msg); });

  auto math_module = okuu_lib["math"].get_or_create<sol::table>();
  // clang-format off
  math_module.new_usertype<cmplx>(
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
  math_module.new_usertype<vec2>(
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
  math_module.new_usertype<mat4>(
    "mat4",
      sol::call_constructor, sol::constructors<mat4()>{},
    sol::meta_function::multiplication,
      sol::resolve<mat4(const mat4&, const mat4&)>(&glm::operator*)
  );
  // clang-format on

  return okuu_lib;
}

struct stage_data {
  sol::optional<sol::protected_function> stage_setup;
  sol::protected_function stage_run;
};

fn setup_package_module(sol::table& okuu_lib, ntf::optional<stage_data>& stage_data) {
  auto package_module = okuu_lib["package"].get_or_create<sol::table>();

  package_module["start_stage"].set_function([&](sol::this_state, sol::table args) {
    auto setup = args.get<sol::optional<sol::protected_function>>("setup");
    auto run = args.get<sol::optional<sol::protected_function>>("run");
    if (!run.has_value()) {
      return;
    }
    stage_data.emplace(std::move(setup), std::move(*run));
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

stage_env::stage_env(stage::stage_scene& scene, sol::state&& lua,
                     sol::optional<sol::protected_function>&& stage_setup,
                     sol::coroutine&& stage_run) :
    _scene{scene},
    _lua{std::move(lua)}, _stage_setup{std::move(stage_setup)}, _stage_run{std::move(stage_run)} {}

static constexpr std::string_view incl_path = ";res/script/?.lua";

expect<stage_env> stage_env::load(const std::string& script_path, stage::stage_scene& scene,
                                  assets::asset_bundle& assets) {
  sol::state lua;
  lua.open_libraries(sol::lib::base, sol::lib::coroutine, sol::lib::package, sol::lib::table,
                     sol::lib::math, sol::lib::string);
  lua["package"]["path"] = incl_path.data();
  auto okuu_lib = lua["okuu"].get_or_create<sol::table>();
  setup_okuu_base(okuu_lib);
  lua_assets::setup_module(okuu_lib, assets);

  ntf::optional<stage_data> stage_data;
  auto package_module = setup_package_module(okuu_lib, stage_data);

  try {
    lua.safe_script_file(script_path);
    clean_package_module(okuu_lib); // stage_data will become a dangling pointer otherwise
    if (!stage_data.has_value()) {
      return {ntf::unexpect, "No stage functions defined in lua scriptl"};
    }
    sol::coroutine run_coro{lua.lua_state(), stage_data->stage_run};

    return {ntf::in_place, scene, std::move(lua), std::move(stage_data->stage_setup),
            std::move(run_coro)};
  } catch (const sol::error& err) {
    return {ntf::unexpect, err.what()};
  }
}

void stage_env::setup_stage_modules() {
  if (!_stage_setup) {
    return;
  }
  auto okuu_lib = _lua["okuu"].get<sol::table>();
  auto env = lua_stage::setup_module(okuu_lib, *this);
  auto setup_res = std::invoke(*_stage_setup, env);
  if (!setup_res.valid()) {
    sol::error err = setup_res;
    logger::error("Error on script stage_setup: {}", err.what());
  }
}

void stage_env::run_tasks() {
  auto env = lua_stage::instance(_lua);
  std::invoke(_stage_run, env);
}

void stage_env::trigger_event(std::string name, sol::variadic_args args) {
  auto it = _events.find(name);
  if (it == _events.end()) {
    return;
  }
  for (auto& event : it->second) {
    std::invoke(event, args);
  }
}

auto stage_env::register_event(std::string name, sol::protected_function func) -> list_iterator {
  auto list_it = _events.find(name);
  if (list_it == _events.end()) {
    auto [it, empl] = _events.try_emplace(std::move(name));
    list_it = it;
  }
  NTF_ASSERT(list_it != _events.end());
  auto& list = list_it->second;
  auto it = list.insert(list.end(), std::move(func));
  return {it};
}

void stage_env::unregister_event(std::string name, list_iterator event) {
  auto it = _events.find(name);
  if (it == _events.end()) {
    return;
  }
  it->second.erase(event);
}

void stage_env::clear_events(std::string name) {
  auto it = _events.find(name);
  if (it == _events.end()) {
    return;
  }
  NTF_ASSERT(it != _events.end());
  it->second.clear();
}

} // namespace okuu::lua

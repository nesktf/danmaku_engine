## Lua api definitions

```cpp
using lua_seq = ...; // This is for using lua_tables as arrays

template<typename T>
using lua_table = ...>; // Pretend this is a lua table

template<typename T>
using lua_array = lua_table<lua_seq, T>; // Pretend this is a sequential lua table

template<typename Signature>
using lua_func = std::function<Signature>; // Pretend this is a lua function

template<typename Signature>
using lua_coro = ...; // Pretend this is a lua function that can yield

#define fn auto // lol

namespace okuu::assets {

enum class type {
    spritesheet = 0,
    audio = 1,
};

struct sprite {

};

struct animation {

};

using renderable = variant<animation, sprite>;

struct any_asset {

};

fn asset_type::load() -> void;
fn asset_type::unload() -> void;

struct spritesheet : any_asset {

};

fn spritesheet::get_sprite(string name) -> optional<sprite>;
fn spritesheet::get_anim(string name) -> optional<animation>;
fn spritesheet::get_rev_anim(string name) -> optional<animation>;

struct audio : any_asset {
};

fn require(string asset_name) throw -> asset_type;
fn assert_loaded(string asset_name) throw -> asset_type;

} // namespace okuu::assets

namespace okuu::package {

struct asset_arg {
    bool is_static;
    string path;
    okuu::assets::type type;
};

fn register_assets(lua_table<asset_arg> args) throw -> lua_table<okuu::assets::any_asset>;

struct lua_package_args {
    struct player_stats {
        f32 vel;
        f32 acc;
        f32 focus;
        f32 hitbox;
    };

    using player_bomb = lua_table<
        <"name", string>,
        <"desc", string>,
        <lua_seq, lua_coro<void(okuu::stage_state)>>
    >;

    struct player_entry {
        string desc;
        renderable card_image;
        lua_func<void(...)> anim_handler;
        player_stats stats;
        lua_array<player_bomb> bombs;
    };
    struct stage_entry {
        string name;
        lua_func<void(okuu::stage_state)> setup;
        lua_coro<void(okuu::stage_state)> run;
        lua_func<void(okuu::stage_state)> on_tick;
        lua_func<void(okuu::stage_state)> on_die;
        lua_func<void(okuu::stage_state)> on_focus;
        lua_func<void(okuu::stage_state)> on_dialog;
    };

    lua_table<
        <string, player_entry>
    > players;

    lua_table<
        <"phantom", optional<stage_entry>>,
        <"extra", optional<stage_entry>>,
        <lua_seq, stage_entry>,
    > stages;
};

fn setup(lua_package_args args) throw -> void;

} // namespace okuu::package

namespace okuu {

using vec = variant<vec2, cmplx>;

struct player_entity {
    vec vel;
    vec pos;
    u32 lives;
    u64 points;
};

fn player_entity::kill() -> void;
fn player_entity::get_ticks() -> u32;
fn player_entity::is_alive() -> bool;

struct projectile_entity {
    vec vel;
    vec pos;
    f32 hitbox;
};

fn projectile_entity::kill() -> void;
fn projectile_entity::get_ticks() -> u32;
fn projectile_entity::is_alive() -> bool;

struct boss_entity {
    vec vel;
    vec pos;
    f32 hitbox;
};

fn boss_entity::kill() -> void;
fn boss_entity::get_ticks() -> u32;
fn boss_entity::is_alive() -> u32;
fn boss_entity::get_slot() -> u32;

struct stage_state {
    enum dialog_side {
        LEFT = 1, RIGHT = 2,
    };
};

fn stage_state::yield_ticks(u32 ticks) yield -> void;
fn stage_state::yield_secs(f32 seconds) yield -> void;
fn stage_state::trigger_dialog(string dialog) -> void;

struct lua_proj_args {

};

fn stage_state::spawn_proj(lua_proj_args args) -> projectile_entity;
fn stage_state::spawn_proj_n(u32 count, lua_func<lua_proj_args(u32)> spawner)
                                                                  -> lua_array<projectile_entity>;

fn stage_state::spawn_boss(lua_boss_args args) throw -> boss_entity;

fn stage_state::get_player() -> player;
fn stage_state::get_boss(u32 slot) -> optional<boss_entity>;
fn stage_state::get_ticks() -> u32;

struct lua_dialog_args {
    struct dialog_entry {
        stage_state::dialog_side side;
        assets::sprite sprite;
        string text;
        optional<bool> inactive;
    };

    using chara_dialogs = lua_table<lua_array<dialog_entry>>;

    lua_table<chara_dialogs>;
};

fn stage_state::setup_dialog(lua_dialog_args args) throw -> void;

} // namespace okuu

```

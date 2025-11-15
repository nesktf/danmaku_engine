local okuu = {}

local DIALOG_LEFT_SIDE = 1
local DIALOG_RIGHT_SIDE = 2

local function stage_setup(state)
  okuu.assets.assert_loaded("chara")

  local chara = okuu.assets.require("chara")
  state:setup_dialog {
    ["chen-start"] = {
      ["cirno"] = {
        {
          side = DIALOG_LEFT_SIDE,
          sprite = chara:get_sprite("cirno0"),
          text = "holy shit a cat"
        },
        {
          side = DIALOG_RIGHT_SIDE,
          sprite = chara:get_sprite("chen0"),
          text = "honk",
        },
        {
          side = DIALOG_LEFT_SIDE,
          sprite = chara:get_sprite("cirno0"),
          text = "i didn't know you were a war criminal",
        },
      },
    },
    ["chen-defeat"] = {
      ["cirno"] = {
        {
          side = DIALOG_RIGHT_SIDE,
          sprite = chara:get_sprite("chen0"),
          inactive = true,
        },
        {
          side = DIALOG_LEFT_SIDE,
          sprite = chara:get_sprite("cirno0"),
          text = "i hate cats",
        },
        {
          side = DIALOG_RIGHT_SIDE,
          sprite = chara:get_sprite("chen0"),
          text = ">:3",
        },
      },
    }
  }
end

local function stage_on_tick(state)
end

local function stage_run(state)
  local chara = okuu.assets.require("chara")

  state.player.pos.x = 0
  state.player.pos.y = 0

  local the_proj = state.projectile:spawn {
    sprite = chara:get_sprite("star_small.black"),
    pos = {x = 20, y = 20},
    vel = {x = 0, y = 1},
  }
  local projs = state.projectile:spawn_n(32, function(n)
    return {
      sprite = chara:get_sprite("star_small.red"),
      pos = {x = 30, y = 30},
      vel = okuu.type.cmplx.polar(32, 2*math.pi/n),
      points = okuu.util.diffval(state.difficulty, 1000, 100, 10, 1),
      state_handler = function(proj)
        proj:yield_ticks(20) -- Activate again in 20 ticks if not dead
        proj.vel = okuu.cmplx.angle_to(proj.pos, state.player.pos)
        proj:yield_ticks(20) -- Activate again in 20 ticks if not dead
        proj.vel = okuu.cmplx.angle_to(proj.pos, state.player.pos)
      end,
    }
  end)
  state:yield_ticks(60)

  if (the_proj:is_alive()) then
    the_proj.pos.x = 0
    the_proj.pos.y = 0
  end
  for _, proj in ipairs(projs) do
    if (proj:is_alive()) then
      proj.pos.x = 20
      proj.pos.y = 30
    end
  end
  state:yield_ticks(60)

  state:trigger_dialog("chen-start")
  local chen = state.boss:spawn {
    pos = {x = 100, y = 100},
    vel = {0, 0},
    state_handler = function(boss)
      state.projectile:spawn_n(32, function(_)
        return {
          sprite = chara:get_sprite("star_small.black"),
          pos = boss.pos,
          vel = {x = 0, y = 1}
        }
      end)
      boss:yield_secs(5)
    end,
    on_die = function(boss)
    end,
  }
  local chen_again = state.boss:get_slot(chen.slot) -- Pointer to chen
  while (chen_again:is_alive()) do
    state:yield()
  end
  state:trigger_dialog("chen-defeat")
end

local assets = okuu.assets.register {
  ["chara"] = { static = true, path = "assets/chara.chima", type = okuu.assets.type.spritesheet },
  ["bg"] = { path = "assets/staking_your_life_on_a_prank.ogg", type = okuu.assets.type.audio, },
  ["pichun"] = { static = true, path = "assets/pichun.ogg", type = okuu.assets.type.audio, },
}

okuu.package.setup {
  players = {
    ["cirno"] = {
      desc = "the strongest",
      card_image = assets.chara:get_sprite("cirno0"),
      anim = okuu.anim.new_player_generic {
        idle = assets.chara:get_anim("chara_cirno.idle"),
        left = assets.chara:get_anim("chara_cirno.left"),
        right = assets.chara:get_anim("chara_cirno.right"),
        idle_left = assets.chara:get_anim("chara_cirno.idle_to_left"),
        left_idle = assets.chara:get_rev_anim("chara_cirno.idle_to_left"),
        idle_right = assets.chara:get_anim("chara_cirno.idle_to_right"),
        right_idle = assets.chara:get_rev_anim("chara_cirno.idle_to_right"),
      },
      stats = {
        vel = 1.0,
        acc = 1.0,
        focus = 0.8,
        hitbox = 3.0,
      },
      bombs = {
        {
          name = "icicle fall",
          desc = "haha funny baka attack",
          function(state)
          end,
        }
      },
    },
    ["marisa"] = {
      desc = "ordinary magician",
      card_image = assets.chara:get_sprite("marisa0"),
      anim = okuu.anim.new_player_generic {
        idle = assets.chara:get_anim("chara_marisa.idle"),
        left = assets.chara:get_anim("chara_marisa.left"),
        right = assets.chara:get_anim("chara_marisa.right"),
        idle_left = assets.chara:get_anim("chara_marisa.idle_to_left"),
        left_idle = assets.chara:get_rev_anim("chara_marisa.idle_to_left"),
        idle_right = assets.chara:get_anim("chara_marisa.idle_to_right"),
        right_idle = assets.chara:get_rev_anim("chara_marisa.idle_to_right"),
      },
      stats = {
        vel = 1.0,
        acc = 1.0,
        focus = 0.8,
        hitbox = 5.0,
      },
      bombs = {
        {
          name = "master spark",
          desc = "marisa throws a fucking laser",
          function(state)
          end,
        }
      },
    },
    ["reimu"] = {
      desc = "the crimson slayer",
      card_image = assets.chara:get_sprite("reimu0"),
      anim = okuu.anim.new_player_generic {
        idle = assets.chara:get_anim("chara_reimu.idle"),
        left = assets.chara:get_anim("chara_reimu.left"),
        right = assets.chara:get_anim("chara_reimu.right"),
        idle_left = assets.chara:get_anim("chara_reimu.idle_to_left"),
        left_idle = assets.chara:get_rev_anim("chara_reimu.idle_to_left"),
        idle_right = assets.chara:get_anim("chara_reimu.idle_to_right"),
        right_idle = assets.chara:get_rev_anim("chara_reimu.idle_to_right"),
      },
      stats = {
        vel = 1.0,
        acc = 1.0,
        focus = 0.8,
        hitbox = 4.0,
      },
      bombs = {
        {
          name = "fantasy seal",
          desc = "reimu seals and betrays you for 1000 years",
          function(state)
          end,
        },
      },
    }
  },
  stages = {
    phantom = nil,
    extra = nil,
    {
      name = "the funny stage",
      setup = stage_setup,
      run = stage_run,
      on_tick = stage_on_tick,
    }
  }
}

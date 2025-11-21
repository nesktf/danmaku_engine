local okuu = okuu

local function stage_setup(_)
  okuu.logger.info("[lua] On stage_setup()!!!")
  okuu.assets.require("chara")
end

local function stage_run(state)
  okuu.logger.info("[lua] On stage_run()!!!")

  local chara = okuu.assets.require("chara")
  local player = state:get_player()
  player:set_pos(0, 0)

  local the_proj = state:spawn_proj {
    sprite = chara:get_sprite("marisa0"),
    pos = { x = 0, y = -200 },
    vel = { x = 0, y = 0},
    scale = { x = 150, y = 150 },
    -- angular_speed = 2*math.pi,
  }
  local sprites = {
    "chara_marisa.idle.1",
    "chara_reimu.idle.1"
  }
  local function move_to(x, y)
    local proj_pos = the_proj:get_pos()
    state:spawn_proj_n(16, function(n)
      local dir_x = 10*math.cos(2*math.pi*n/16)
      local dir_y = 10*math.sin(2*math.pi*n/16)
      return {
        sprite = chara:get_sprite("star_med.1"),
        pos = { x = proj_pos.x, y = proj_pos.y },
        vel = { x = 0, y = 0},
        scale = { x = 50, y = 50 },
        angular_speed = 2*math.pi,
        movement = okuu.stage.movement.move_linear(dir_x, dir_y)
      }
    end)
    state:spawn_proj_n(32, function(n)
      local player_pos = player:get_pos()
      local dir_player = player_pos - proj_pos
      local len = math.sqrt(dir_player.x*dir_player.x + dir_player.y*dir_player.y)

      local ang = math.random()/2
      local sp = math.random(4, 10)
      local dir = okuu.math.cmplx(sp*dir_player.x/len, sp*dir_player.y/len)
      if (n % 2 == 0) then
        local rot = okuu.math.cmplx(math.cos(ang), math.sin(-ang))
        dir = dir * rot
      else
        local rot = okuu.math.cmplx(math.cos(ang), math.sin(ang))
        dir = dir * rot
      end

      return {
        sprite = chara:get_sprite(sprites[n%2 == 0 and 1 or 2]),
        pos = { x = proj_pos.x, y = proj_pos.y },
        vel = { x = 0, y = 0},
        scale = { x = 50, y = 50 },
        angular_speed = 2*math.pi,
        movement = okuu.stage.movement.move_linear(dir.real, dir.imag)
      }
    end)
    the_proj:set_movement(okuu.stage.movement.move_towards(10., 10., x, y))
  end

  while (true) do
    move_to(-150, -250)
    state:yield_secs(.5)
    move_to(150, -250)
    state:yield_secs(.5)
  end
end

okuu.package.start_stage {
  setup = stage_setup,
  run = stage_run,
}

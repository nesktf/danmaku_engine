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
    sprite = chara:get_sprite("star_med.15"),
    pos = { x = 0, y = -200 },
    vel = { x = 0, y = 0},
    scale = { x = 50, y = 50 },
    angular_speed = 2*math.pi,
  }
  while (true) do
    state:yield_secs(.5)
    local player_pos = player:get_pos()
    local proj_pos = the_proj:get_pos()
    the_proj:set_movement(okuu.stage.movement.move_towards(10., 10., player_pos.x, player_pos.y))
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
  end
end

okuu.package.start_stage {
  setup = stage_setup,
  run = stage_run,
}

local okuu = okuu

local function stage_setup(stage)
  okuu.logger.info("[lua] On stage_setup()!!!")
  okuu.assets.require("chara")
  stage:register_event("stage::on_boss_move", function(pos)
    okuu.logger.info(string.format("[lua] Marisa moved to %f %f!!!!", pos.x, pos.y))
  end)
end

local function stage_run(stage)
  okuu.logger.info("[lua] On stage_run()!!!")

  local chara = okuu.assets.require("chara")
  local player = stage:get_player()
  player:set_pos(0, 0)

  local marisa_sprite = chara:get_sprite("marisa0")
  local marisa_boss = stage:spawn_proj {
    sprite = marisa_sprite,
    pos = { x = 0, y = -200 },
    vel = { x = 0, y = 0},
    scale = { x = 150, y = 150 },
  }
  local marisa = stage:spawn_sprite {
    sprite = marisa_sprite,
    pos = { x = 200, y = 200 },
    vel = { x = 0, y = 0},
    scale = { x = 500, y = 500 },
  }

  local cirno_sprite = chara:get_sprite("cirno0")
  local cirno = stage:spawn_sprite {
    sprite = cirno_sprite,
    pos = { x = -200, y = 200 },
    vel = { x = 0, y = 0},
    scale = { x = -500, y = 500 },
  }

  local reimu_sprite = chara:get_sprite("reimu0")
  local reimu = stage:spawn_sprite {
    sprite = reimu_sprite,
    pos = { x = -800, y = -150 },
    vel = { x = 0, y = 0},
    scale = { x = -500, y = 500 },
  }

  local sprites = {
    "chara_marisa.idle.1",
    "chara_reimu.idle.1"
  }
  local function move_to(x, y)
    local proj_pos = marisa_boss:get_pos()
    stage:spawn_proj_n(16, function(n)
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
    stage:spawn_proj_n(32, function(n)
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
    marisa_boss:set_movement(okuu.stage.movement.move_towards(10., 10., x, y))
    stage:trigger_event("stage::on_boss_move", {x = proj_pos.x, y = proj_pos.y})
  end

  while (true) do
    reimu:set_movement(okuu.stage.movement.move_linear(20, 0))
    move_to(-150, -250)
    stage:yield_secs(.5)
    move_to(150, -250)
    stage:yield_secs(.5)
    reimu:set_pos(-600, -150)
  end
end

okuu.package.start_stage {
  setup = stage_setup,
  run = stage_run,
}

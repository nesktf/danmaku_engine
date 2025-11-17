local okuu = okuu

local function stage_setup(_)
  okuu.logger.info("[lua] On stage_setup()!!!")
  okuu.assets.require("chara")
end

local function stage_run(state)
  okuu.logger.info("[lua] On stage_run()!!!")

  local chara = okuu.assets.require("chara")
  local player = state:get_player()
  player.pos.x = 0
  player.pos.y = 0

  local the_proj = state:spawn_proj {
    sprite = chara:get_sprite("star_med.0"),
    pos = { x = 0, y = 500 },
    vel = { x = 0, y = 0},
  }
  while (true) do
    okuu.logger.info("[lua] Yielding 5 secs!!!")
    state:yield_secs(5)
    the_proj.pos.x = player.pos.x
    the_proj.pos.y = player.pos.y
  end
end

okuu.package.start_stage {
  setup = stage_setup,
  run = stage_run,
}

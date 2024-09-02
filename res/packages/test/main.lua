local task = require("task")
local log = require("log")
local cmplx = require("cmplx")
local global = __GLOBAL

local pi = math.pi

local function request() 
  res_request_atlas("chara", "res/spritesheet/chara.json")
  res_request_atlas("effects", "res/spritesheet/effects.json")
  res_request_atlas("enemies", "res/spritesheet/enemies.json")

  res_request_shader("fairy_shader", "...")

  res_request_font("funny_font", "...")
end

local sprites = {}
local function setup()
  local atlas = res_get_atlas("effects")
  local shader = res_get_shader("fairy_shader")

  sprites["stars_small"] = atlas:get_group("stars_small")
  sprites["stars_big"] = atlas:get_group("stars_big")

  set_player_anim("idle", atlas:get_sequence("marisa.idle"))
  set_player_anim("move", atlas:get_sequence("marisa.move"))

  set_boss_anim("idle", atlas:get_sequence("chen.idle"))
  set_boss_anim("move", atlas:get_sequence("chen.move"))

  local my_danmaku = {
    renderer = function(self)
      self:set_sprite_sequence(atlas:get_sequence("stars.sparks"))
    end,
    pos = cmplx.new(0, 0),
    scale = 2,
    task = function(self)
      self:set_movement(movement_towards(cmplx.new(1, 1), cmplx.new(1, 1), cmplx.new(1, 0), 1.0))
      coroutine.yield()
    end,
    on_death = function(self)
    end
  }

  local my_fairy = {
    shader = shader,
    renderer = function(self)
      local the_shader = self:get_shader()
      local speed = self:get_speed()
      if (clen(speed) > 20) then
        the_shader:set_uniform("color", vec3.new(255, 255, 255))
        self.sprite = sprites["stars_small"]
      else
        the_shader:set_uniform("color", vec3.new(0, 0, 0))
        self.sprite = sprites["stars_big"]
      end
    end,
    life = 5,
    pos = cmplx.new(2, 3),
    scale = 20,
    task = function(self)
      local lifetime = self:get_lifetime()

      if (lifetime < ups*10) then
        spawn_danmaku(my_danmaku)
      else
        spawn_danmaku(my_danmaku)
      end

      coroutine.yield()
    end,
    on_death = function(self)
    end
  }

  spawn_enemy(my_fairy)
end

local function init()
  log.debug("[lua::init] on init task!!!! ups = %d, dt = %f", global.ups, global.dt)
end

local function move_to_player()
  local player_pos = __PLAYER_POS()
  __MOVE_BOSS(cmplx.new(player_pos.x, player_pos.y))
end

local function shoot_danmaku()
  local boss_pos = __BOSS_POS()
  local count = 16
  for i=0,count-1 do 
    local dir = cmplx.expi(2*pi*i/count)
    __SPAWN_DANMAKU(dir, 5.0, boss_pos)
  end
end

local function main()
  local vp = __VIEWPORT()
  __SPAWN_BOSS(50.0, pi, cmplx.new(-10, -10), cmplx.new(vp.x*0.5, vp.y*0.25))
  task.wait(60)

  while (true) do
    task.invoke_delayed(20, function()
      move_to_player()
      shoot_danmaku()
    end)
    task.invoke_delayed(20, shoot_danmaku)
  end
end

task.setup_stage({
  init_task = init,
  main_task = main,
})

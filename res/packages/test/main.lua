local task = require("task")
local log = require("log")
local cmplx = require("cmplx")
local global = __GLOBAL

local pi = math.pi

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

local function funny_danmaku()
  local list = funny_lib.pview(32)
  local count = list:size()
  local i = 0
  list:for_each(function(proj)
    local dir = cmplx.expi(2*pi*i/count)
    dir.real = dir.real*10
    dir.imag = dir.imag*10
    proj.movement = funny_lib.move.make_linear(dir)
    i = i+1
  end)
  task.wait(30)
  i = 0
  list:for_each(function(proj)
    local dir = cmplx.expi(2*pi*i/count)
    dir.real = dir.real*-10
    dir.imag = dir.imag*-10
    proj.movement = funny_lib.move.make_linear(dir)
    i = i+1
  end)
end

local function main()
  local vp = __VIEWPORT()
  __SPAWN_BOSS(50.0, pi, cmplx.new(-10, -10), cmplx.new(vp.x*0.5, vp.y*0.25))
  task.wait(60)

  while (true) do
    task.invoke_delayed(20, function()
      move_to_player()
      shoot_danmaku()
      funny_danmaku()
    end)
    task.invoke_delayed(20, shoot_danmaku)
  end
end

task.setup_stage({
  init_task = init,
  main_task = main,
})

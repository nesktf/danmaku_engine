local task = require("task")
local log = require("log")
local global = __GLOBAL

local dt = global.dt
local ups = global.ups

local function init()
  log.debug("[lua::init] on init task!!!! ups = %d, dt = %f", ups, dt)
end

local function main()
  log.debug("[lua::main] on main task!! I will wait 200 ticks!!!!")
  task.wait(200)
  log.debug("[lua::main] waited 20 ticks!!! now I will die!!!!!")
end

task.setup_stage({
  init_task = init,
  main_task = main,
})

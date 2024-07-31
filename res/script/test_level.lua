local log_debug = log_debug
local create_bullet = create_bullet

local task_coroutine = nil

local dt = 1.0/60.0

local function level_task()
  while true do
    log_debug("coroutine1")
    create_bullet(-150*dt, 100*dt, 80*dt*(1-0.984), -150*dt*(1-0.984), 0.984)
    coroutine.yield(40)
    log_debug("coroutine2")
    create_bullet(0, -10*dt, 0, -3*dt, 1)
    coroutine.yield(20)
  end
end

function NEXT_TASK()
  local wait_time = 0
  if (task_coroutine and coroutine.status(task_coroutine) ~= 'dead') then
    local _, wait = coroutine.resume(task_coroutine)
    wait_time = wait
  end
  return wait_time
end

function INIT()
  log_debug("test!!")
  task_coroutine = coroutine.create(level_task)
end

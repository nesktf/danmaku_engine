#!/usr/bin/env lua5.3
local coroutine = require("coroutine")
local math = require("math")
-- state lib?

-- task lib
local task = {}
local state = {
  timer = 0,
  fun = nil
}

function task.wait(time)
  coroutine.yield({wait_time = time})
end

function task.wait_event(event)
  coroutine.yield({wait_event = event})
end

function task.yield()
  coroutine.yield({wait_time = 0})
end

function task.invoke_delayed(delay, sub_task, ...)
  if (delay > 0) then
    task.wait(delay)
  end
  return sub_task(...)
end

function task.invoke_on_event(event, sub_task, ...)
  task.wait_event(event)
  return sub_task(...)
end

function task.invoke_on_tick(tick, sub_task, ...)
  task.wait(tick-state.timer)
  return sub_task(...)
end

function task.setup_main_task(fun)
  state.fun = fun
end

function task.__invoke_main_task()
  -- Test thing, replace with cpp callable
  local task_coroutine = coroutine.create(state.fun)
  print(" -- Coroutine start")
  while (coroutine.status(task_coroutine) ~= 'dead') do
    local _, task_yield = coroutine.resume(task_coroutine)

    if (coroutine.status(task_coroutine) == 'dead') then
      print(" -- Coroutine ended at "..tostring(state.timer).." ticks")
      break
    end

    if (task_yield.wait_event ~= nil) then
      print(" -- Coroutine yielded wait_event: "..task_yield.wait_event)
      state.timer = state.timer + math.random(20, 120)
    else
      print(" -- Coroutine yieleded wait_time: "..task_yield.wait_time)
      state.timer = state.timer + task_yield.wait_time
    end
  end
end

-- Stage things
local function stage_timeline()
  local function log_task(msg)
    print(msg.." -> state.timer = "..tostring(state.timer))
  end

  local function spawn_danmaku(count)
    print("Spawning "..tostring(count).." units of danmaku!!!!")
  end

  local function spawn_fairy()
    print("Spawning fairy, wait 20 again!!")
    task.invoke_delayed(20, spawn_danmaku, 10)
  end

  local function spawn_boss()
    print("Spawning chen, wait 30 ticks again!!!")
    task.invoke_delayed(30, function(c)
      print("Atacking!!!!")
      spawn_danmaku(c)
    end, 40)
  end

  local function end_level()
    print("CHEEEEEEEEEEEEEEEEEEN")
  end

  local function kill_player()
    print("PICHUUUUUUUUUUUUN")
  end

  log_task("Stage start!!!")

  log_task("Task: spawn_fairy in 20 ticks!!")
  task.invoke_delayed(20, spawn_fairy)

  log_task("Wait: 16 before spawning boss!!")
  task.wait(16)

  log_task("Task: spawn_boss in 30 ticks!!")
  task.invoke_delayed(30, spawn_boss)

  log_task("Task: end_level on event boss_defated!!!")
  task.invoke_on_event("boss_defeated", end_level)

  log_task("Task: kill_player on tick 250!!!")
  task.invoke_on_tick(250, kill_player)

  log_task("Stage end!!!")
end
task.setup_main_task(stage_timeline)

-- Entrypoint
task.__invoke_main_task()

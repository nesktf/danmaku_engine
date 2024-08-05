local coroutine = require("coroutine")

local _M = {}

function _M.wait(time)
  if (time <= 0) then
    return
  end
  coroutine.yield({wait_time = time})
end

function _M.wait_event(event)
  coroutine.yield({wait_event = event})
end

function _M.yield()
  coroutine.yield({wait_time = 0})
end

function _M.invoke_delayed(delay, sub_task, ...)
  _M.wait(delay)
  return sub_task(...)
end

function _M.invoke_on_event(event, sub_task, ...)
  _M.wait_event(event)
  return sub_task(...)
end

function _M.setup_stage(args)
  assert(args.main_task ~= nil, "No main task provided")

  local init = args.init_task
  if (args.init_task ~= nil) then
    __INIT_TASK = function() init() end
  end

  local main = coroutine.create(args.main_task)
  __MAIN_TASK = function()
    assert(coroutine.status(main) ~= 'dead', "Task is dead")
    local _, yield = coroutine.resume(main)

    if (coroutine.status(main) == 'dead') then
      -- task ended
      return {wait_time = -1}
    end

    if (yield.wait_event ~= nil) then
      -- task waits for event
      return {wait_time = 0, wait_event = yield.wait_event}
    end

    -- task waits for time
    assert(yield.wait_time ~= nil, "Invalid task time")
    return {wait_time = yield.wait_time}
  end
end

return _M

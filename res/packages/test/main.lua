local okuu = okuu
local log = okuu.log
local stage = okuu.stage
local cmplx = okuu.math.cmplx

function okuu.init()
  log.debug("[lua] On init task!!!")
end

function okuu.main()
  local vp = stage.viewport()
  log.debug(string.format("[lua] On main task!!! vp = %d, %d", vp.x, vp.y))

  local function funny_danmaku()
    local pi = math.pi

    -- Shoot and forget danmaku (stateless (tm))
    local boss_pos = stage.boss_pos()
    local count = 16
    for i=0,count-1 do 
      local dir = cmplx.expi(2*pi*i/count)
      stage.spawn_danmaku(dir, 5.0, boss_pos)
    end

    -- Projectile view danmaku (stateful)
    local list = stage.pview.make(32)
    count = list:size()
    local i = 0
    list:for_each(function(proj)
      local dir = cmplx.expi(2*pi*i/count)
      dir.real = dir.real*10
      dir.imag = dir.imag*10
      proj.movement = stage.move.make_linear(dir)
      i = i+1
    end)
    stage.cowait(30)
    i = 0
    list:for_each(function(proj)
      local dir = cmplx.expi(2*pi*i/count)
      dir.real = dir.real*-10
      dir.imag = dir.imag*-10
      proj.movement = stage.move.make_linear(dir)
      i = i+1
    end)
  end

  stage.spawn_boss(50.0, cmplx(-10, -10), cmplx(vp.x*0.5, vp.y*0.25))
  stage.cowait(60)

  while (true) do
    stage.cowait(20)
    stage.move_boss(stage.player_pos())
    funny_danmaku()
  end
end

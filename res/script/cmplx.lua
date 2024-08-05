local _M = {}
local _meta = {}

function _M.new(real, imag)
  return setmetatable({real = real, imag = imag}, _meta)
end

function _M.cast(val)
  if (type(val) == "table") then
    -- If cmplx
    if (getmetatable(val) == _meta) then
      return val
    end
    -- If array
    local real, imag = tonumber(val[1]), tonumber(val[2])
    return setmetatable({real = real, imag = imag}, _meta)
  end

  -- Assume is number
  local num = tonumber(val)
  assert(num ~= nil, "Invalid cmplx arg")
  return setmetatable({real = num, imag = 0}, _meta)
end

function _M.expi(theta)
  return setmetatable({real = math.cos(theta), imag = math.sin(theta)}, _meta)
end

function _meta.__add(z1, z2)
  local a, b = _M.cast(z1), _M.cast(z2)
  return _M.new(a.real+b.real, a.imag+b.imag)
end

function _meta.__sub(z1, z2)
  local a, b = _M.cast(z1), _M.cast(z2)
  return _M.new(a.real-b.real, a.imag-b.imag)
end

function _meta.__mul(z1, z2)
  local a, b = _M.cast(z1), _M.cast(z2)
  return _M.new(a.real*b.real-a.imag*b.imag, a.real*b.imag+a.imag*b.real)
end

return _M

local chima = require("chimatools")
local inspect = require("inspect")
local ctx = chima.context.new()
local ffi = require("ffi")

local function copy_image(width, height, dst, src, src_x, src_y)
  local src_data = ffi.cast("uint8_t*", src.data)
  local dst_data = ffi.cast("uint8_t*", dst.data)

  for y = 0, (height - 1) do
    for x = 0, (width - 1) do
      local src_pixel_pos = ((((y+src_y) * src.width) + x + src_x) * src.channels)
      local src_pixel = (src_data + src_pixel_pos)

      local dst_pixel_pos = (((y * dst.width) + x) * src.channels)
      local dst_pixel = (dst_data + dst_pixel_pos)
      dst_pixel[0] = src_pixel[0]
      dst_pixel[1] = src_pixel[1]
      dst_pixel[2] = src_pixel[2]
      dst_pixel[3] = src_pixel[3]
    end
  end
end

local function load_animation(chara_data)
  -- TODO: Add this functionality to the chimatools luajit FFI interface
  local name = assert(chara_data.name)
  local image_path = assert(chara_data.path)
  local width = assert(chara_data.frame_width)
  local height = assert(chara_data.frame_height)
  local frame_map = assert(chara_data.frames)

  local color = chima.color.new(1, 1, 1, 1)
  local image = chima.image.load(ctx, nil, image_path)
  local cols = math.floor(image.width/width)
  local rows = math.floor(image.height/height)

  local function make_chima_anim(anim_name)
    local chima_anim = ffi.new(chima.anim._ctype)
    chima_anim.ctx = ctx
    ffi.copy(chima_anim.name.data, anim_name, anim_name:len())
    chima_anim.name.length = anim_name:len()
    chima_anim.image_count = 0
    chima_anim.fps = 10.0
    chima_anim.images = nil
    return chima_anim
  end

  local anim_suffixes = {
    [1] = "idle",
    [2] = "left",
    [3] = "idle_to_left",
    [4] = "right",
    [5] = "idle_to_right",
  }
  local frames_groups = {
    [1] = { count = 0, anim = make_chima_anim(name.."."..anim_suffixes[1]), frames = {} },
    [2] = { count = 0, anim = make_chima_anim(name.."."..anim_suffixes[2]), frames = {} },
    [3] = { count = 0, anim = make_chima_anim(name.."."..anim_suffixes[3]), frames = {} },
    [4] = { count = 0, anim = make_chima_anim(name.."."..anim_suffixes[4]), frames = {} },
    [5] = { count = 0, anim = make_chima_anim(name.."."..anim_suffixes[5]), frames = {} },
  }

  local frame_count = 0
  for row = 1, rows do
    local y = (row-1)*height  
    for col = 0, cols-1 do
      local x = col*width
      local frame_type = frame_map[frame_count+1]
      local frame_num = tostring(frames_groups[frame_type].count)
      local sprite_name = name .. "." .. anim_suffixes[frame_type].."."..frame_num
      frames_groups[frame_type].count = frames_groups[frame_type].count +1

      local sprite_img = chima.image.new(ctx, width, height, 4, color, sprite_name)
      copy_image(width, height, sprite_img, image, x, y)
      sprite_img.anim = frames_groups[frame_map[frame_count+1]].anim

      table.insert(frames_groups[frame_map[frame_count+1]].frames, sprite_img)
      frame_count = frame_count + 1
    end
  end

  for _, group in ipairs(frames_groups) do
    group._array = ffi.new("chima_image[?]", #group.frames)
    for i = 0, group.count - 1 do
      ffi.copy(group._array[i], group.frames[i+1], ffi.sizeof("chima_image"))
    end
    group.anim.images = ffi.cast("chima_image*", group._array)
    group.anim.image_count = group.count
  end

  return frames_groups
end

local default_frame_map = {
  1, 1, 1, 1, 1, 1, 1, 1, -- idle
  3, 3, 3, 3, -- idle_to_left
  2, 2, 2, 2, -- left
  5, 5, 5, 5, -- idle_to_right
  4, 4, 4, 4, -- right
}

local marisa_data = {
  name = "chara_marisa",
  path = "./mari_movs.png",
  frame_width = 32,
  frame_height = 48,
  frames = default_frame_map,
}

local reimu_data = {
  name = "chara_reimu",
  path = "./remu_movs.png",
  frame_width = 32,
  frame_height = 48,
  frames = default_frame_map,
}

local cirno_data = {
  name = "chara_cirno",
  path = "./cirno_movs.png",
  frame_width = 32,
  frame_height = 48,
  frames = default_frame_map,
}

local images = {
  {name = "marisa0", path = "./flani_mari.png"},
  {name = "reimu0", path = "./flani_remu.png"},
}
local padding = 2
local output = "chara"

do
  local all_frames = {}
  local anims = {}
  for _, data in ipairs({marisa_data, reimu_data, cirno_data}) do
    local frames = load_animation(data)
    table.insert(all_frames, frames)
    for _, frame in ipairs(frames) do
      table.insert(anims, frame.anim)
    end
  end
  local imgs = {}
  for _, image_data in ipairs(images) do
    local img = chima.image.load(ctx, image_data.name, image_data.path)
    table.insert(imgs, img)
  end
  local sheet = chima.spritesheet.new(ctx, padding, imgs, anims)
  print(inspect(sheet))
  for i = 0, sheet.sprite_count-1 do
    local sprite = sheet.sprites[i]
    local bot_right_x = math.floor(((sprite.uv_x_lin/sheet.atlas.width) + sprite.uv_x_con)*sheet.atlas.width)
    local bot_right_y = math.floor(((sprite.uv_y_lin/sheet.atlas.height) + sprite.uv_y_con)*sheet.atlas.height)
    print(string.format("- %s - ends at (%d, %d), sz (%d x %d)",
      sprite.name, bot_right_x, bot_right_y, sprite.width, sprite.height))
  end

  sheet.atlas:write(output..".png", chima.image.format.png)
  sheet:write(output..".chima")
end

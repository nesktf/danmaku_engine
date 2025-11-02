local chima = require("chimatools")
local inspect = require("inspect")
local ffi = require("ffi")
local cjson = require("cjson")

local ctx = chima.context.new()

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

local function frames_to_anim(anim_name, fps, frames, frame_count)
  -- TODO: Add this functionality to the chimatools luajit FFI interface
  local chima_anim = ffi.new(chima.anim._ctype)
  chima_anim.ctx = ctx
  ffi.copy(chima_anim.name.data, anim_name, anim_name:len())
  chima_anim.name.length = anim_name:len()
  chima_anim.image_count = 0
  chima_anim.fps = fps
  chima_anim.images = nil

  for i, frame in ipairs(frames) do
    local frame_name = anim_name.."."..tostring(i)
    ffi.copy(frame.name.data, frame_name, frame_name:len())
    frame.name.length = frame_name:len()
    frame.anim = chima_anim
  end

  local _array = ffi.new("chima_image[?]", frame_count)
  for i = 0, frame_count - 1 do
    ffi.copy(_array[i], frames[i+1], ffi.sizeof("chima_image"))
  end
  chima_anim.images = ffi.cast("chima_image*", _array)
  chima_anim.image_count = frame_count

  return chima_anim, _array
end

local function load_animation(chara_data)
  local name = assert(chara_data.name)
  local image_path = assert(chara_data.path)
  local width = assert(chara_data.frame_width)
  local height = assert(chara_data.frame_height)
  local frame_map = assert(chara_data.frames)
  local fps = assert(chara_data.fps)

  local color = chima.color.new(1, 1, 1, 1)
  local image = chima.image.load(ctx, nil, image_path)
  local cols = math.floor(image.width/width)
  local rows = math.floor(image.height/height)


  local anim_groups = {
    [1] = { frames = {}, count = 0, anim = nil, suffix = "idle" },
    [2] = { frames = {}, count = 0, anim = nil, suffix = "left" },
    [3] = { frames = {}, count = 0, anim = nil, suffix = "idle_to_left" },
    [4] = { frames = {}, count = 0, anim = nil, suffix = "right" },
    [5] = { frames = {}, count = 0, anim = nil, suffix = "idle_to_right" },
  }

  local frame_count = 0
  for row = 1, rows do
    local y = (row-1)*height  
    for col = 0, cols-1 do
      local x = col*width
      local anim_type = frame_map[frame_count+1]
      local group = anim_groups[anim_type]

      local frame_num = tostring(group.count)
      local sprite_name = name .. "." .. group.suffix .."."..frame_num
      group.count = group.count + 1

      local sprite_img = chima.image.new(ctx, width, height, 4, color, sprite_name)
      copy_image(width, height, sprite_img, image, x, y)
      table.insert(group.frames, sprite_img)
      frame_count = frame_count + 1
    end
  end

  for _, group in ipairs(anim_groups) do
    local anim_name = name.."."..group.suffix
    local anim, _array = frames_to_anim(anim_name, fps, group.frames, group.count)
    group.anim = anim
    group._array = _array
  end

  return anim_groups
end

local function parse_old_sheet(json_path, image_path)
  local json_content
  do
    local file = assert(io.open(json_path, "r"))
    json_content = file:read("*all")
    file:close()
  end
  local sheet_image = chima.image.load(ctx, nil, image_path)
  local json = cjson.decode(json_content).content

  local color = chima.color.new(1, 1, 1, 1)
  local anims_out = {}
  local images_out = {}

  for _, elem in ipairs(json) do
    local curr_images = {}
    local base_name = elem.name
    local anims = elem.anim
    local x0 = elem.offset.x0
    local y0 = elem.offset.y0
    local image_count = elem.offset.count

    local cols = elem.offset.cols
    local rows = math.ceil(image_count/cols)

    local dy = math.floor(elem.offset.dy/rows)
    local dx = math.floor(elem.offset.dx/cols)

    for i = 0, image_count-1 do
      local sprite_name = base_name.."."..tostring(i)
      local row = math.floor(i/cols)
      local col = i % cols
      local image = chima.image.new(ctx, dx, dy, 4, color, sprite_name)
      copy_image(dx, dy, image, sheet_image, x0+(dx*col), y0+(dy*row))
      table.insert(curr_images, image)
    end

    if (#anims > 0) then
      for _, anim in ipairs(anims) do
        local anim_name = base_name.."."..anim.name
        local seq = anim.sequence
        local fps = anim.delay*#seq

        local anim_frames = {}
        for _, idx in ipairs(seq) do
          table.insert(anim_frames, curr_images[idx+1])
          curr_images[idx+1] = nil
        end
        local frame_anim, _array = frames_to_anim(anim_name, fps, anim_frames, #anim_frames)
        table.insert(anims_out, {
          count = #anim_frames,
          frames = anim_frames,
          anim = frame_anim,
          _array = _array,
        })
      end
    end

    for _, image in pairs(curr_images) do
      table.insert(images_out, image)
    end
  end
  return images_out, anims_out
end

local function collect_anims(anims)
  local anims_out = {}
  for _, anim in ipairs(anims) do
    table.insert(anims_out, anim.anim)
  end
  return anims_out
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
  path = "../temp/sprite_thing/mari_movs.png",
  fps = 10,
  frame_width = 32,
  frame_height = 48,
  frames = default_frame_map,
}

local reimu_data = {
  name = "chara_reimu",
  path = "../temp/sprite_thing/remu_movs.png",
  fps = 10,
  frame_width = 32,
  frame_height = 48,
  frames = default_frame_map,
}

local cirno_data = {
  name = "chara_cirno",
  path = "../temp/sprite_thing/cirno_movs.png",
  fps = 10,
  frame_width = 32,
  frame_height = 48,
  frames = default_frame_map,
}

local images = {
  {name = "marisa0", path = "../temp/sprite_thing/flani_mari.png"},
  {name = "reimu0", path = "../temp/sprite_thing/flani_remu.png"},
}
local padding = 2
local output_chara = "chara"
local output_effects = "effects"

do
  local imgs, anims = parse_old_sheet("../res/spritesheet/effects.json", "../res/spritesheet/effects.png")
  local sheet = chima.spritesheet.new(ctx, padding, imgs, collect_anims(anims))
  print(inspect(sheet))
  sheet.atlas:write(output_effects..".png", chima.image.format.png)
  sheet:write(output_effects..".chima")
end

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

  sheet.atlas:write(output_chara..".png", chima.image.format.png)
  sheet:write(output_chara..".chima")
end

local okuu = okuu

okuu.package.register_assets {
  ["chara"] = {
    path = "chara.chima",
    type = okuu.assets.type.sprite_atlas,
  },
}

okuu.package.register_stages {
  {
    name = "the funny_stage",
    path = "stage0.lua",
  },
}

okuu.package.register_players {
  ["cirno"] = {
    desc = "the strongest",
    anim_sheet = "chara",
    anim = {
      {"chara_cirno.idle", false},
      {"chara_cirno.left", false},
      {"chara_cirno.idle_to_left", true},
      {"chara_cirno.idle_to_left", false},
      {"chara_cirno.right", false},
      {"chara_cirno.idle_to_right", true},
      {"chara_cirno.idle_to_right", false},
    },
    stats = {
      vel = 1.0,
      acc = 1.0,
      focus = 0.8,
      hitbox = 3.0,
    },
    bombs = {
      {
        name = "icicle fall",
        desc = "haha funny baka attack",
        function(state)
        end,
      }
    },
  },
  ["marisa"] = {
    desc = "ordinary magician",
    anim_sheet = "chara",
    anim = {
      {"chara_marisa.idle", false},
      {"chara_marisa.left", false},
      {"chara_marisa.idle_to_left", true},
      {"chara_marisa.idle_to_left", false},
      {"chara_marisa.right", false},
      {"chara_marisa.idle_to_right", true},
      {"chara_marisa.idle_to_right", false},
    },
    stats = {
      vel = 1.0,
      acc = 1.0,
      focus = 0.8,
      hitbox = 5.0,
    },
    bombs = {
      {
        name = "master spark",
        desc = "marisa throws a fucking laser",
        function(state)
        end,
      }
    },
  },
  ["reimu"] = {
    desc = "the crimson slayer",
    anim_sheet = "chara",
    anim = {
      {"chara_reimu.idle", false},
      {"chara_reimu.left", false},
      {"chara_reimu.idle_to_left", true},
      {"chara_reimu.idle_to_left", false},
      {"chara_reimu.right", false},
      {"chara_reimu.idle_to_right", true},
      {"chara_reimu.idle_to_right", false},
    },
    stats = {
      vel = 1.0,
      acc = 1.0,
      focus = 0.8,
      hitbox = 4.0,
    },
    bombs = {
      {
        name = "fantasy seal",
        desc = "reimu seals and betrays you for 1000 years",
        function(state)
        end,
      },
    },
  }
}

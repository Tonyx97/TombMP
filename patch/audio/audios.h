#pragma once

using audio_id = unsigned int;
using audio_flags = unsigned int;

enum AudioFlag : audio_flags
{
	AUDIO_FLAG_NORMAL = 0x67d1ff66,
	AUDIO_FLAG_LOOPED = 0xf7902bcd,
	AUDIO_FLAG_ONE_SHOT_WAIT = 0xd3d7fed,
	AUDIO_FLAG_ONE_SHOT_REWIND = 0xc233cbae,
};

enum SfxType
{
	SFX_LANDANDWATER	= 0 << 14,
	SFX_LANDONLY		= 1 << 14,
	SFX_WATERONLY		= 2 << 14
};

enum AudioId : audio_id
{
	AUDIO_LARA_FEET = 0xddff6a77,
	AUDIO_LARA_CLIMB2 = 0x63da6a08,
	AUDIO_LARA_NO = 0xbc7025ed,
	AUDIO_LARA_SLIPPING = 0x347bb85e,
	AUDIO_LARA_LAND = 0xac7e8e6e,
	AUDIO_LARA_CLIMB1 = 0x6719f6ab,
	AUDIO_LARA_DRAW = 0x6ba74806,
	AUDIO_LARA_HOLSTER = 0xd4f9013d,
	AUDIO_LARA_FIRE = 0x14dc6709,
	AUDIO_LARA_RELOAD = 0x7602b178,
	AUDIO_LARA_RICOCHET = 0x3941303b,
	AUDIO_LARA_FLARE_IGNITE = 0x848e81a7,
	AUDIO_LARA_FLARE_BURN = 0x66bb78d2,
	AUDIO_LARA_FLARE_IGNITE_W = 0xcce45797,
	AUDIO_LARA_FLARE_BURN_W = 0xf004f675,
	AUDIO_LARA_HARPOON_FIRE = 0x416fb9d8,
	AUDIO_LARA_HARPOON_LOAD = 0x898c9455,
	AUDIO_LARA_WET_FEET = 0x6115808f,
	AUDIO_LARA_WADE = 0x2479be17,
	AUDIO_RUMBLE_LOOP = 0x94c92922,
	AUDIO_ICKET_BARRIER = 0xc972c394,
	AUDIO_CRICKET_LOOP = 0x1d87d9ff,
	AUDIO_LARA_HARPOON_LOAD_WATER = 0xab94a090,
	AUDIO_LARA_HARPOON_FIRE_WATER = 0x2be75dd3,
	AUDIO_LARA_KNEES_SHUFFLE = 0x358b3d9,
	AUDIO_PUSH_SWITCH = 0x2f274cb7,
	AUDIO_LARA_CLIMB3 = 0x1ebcdc6e,
	AUDIO_LARA_BODYSL = 0x5b32319,
	AUDIO_LARA_SHIMMY = 0xadd58671,
	AUDIO_LARA_JUMP = 0xb83ab1d7,
	AUDIO_LARA_FALL = 0xb5b9edd0,
	AUDIO_LARA_INJURY = 0x1ec782b8,
	AUDIO_LARA_ROLL = 0x94d4171d,
	AUDIO_LARA_SPLASH = 0xb8980c81,
	AUDIO_LARA_GETOUT = 0x1a4df622,
	AUDIO_LARA_SWIM = 0x24b9e27f,
	AUDIO_LARA_BREATH = 0x34074039,
	AUDIO_LARA_BUBBLES = 0x2bf28360,
	AUDIO_LARA_SWITCH = 0xe7fd1f4a,
	AUDIO_LARA_KEY = 0xb48eed9b,
	AUDIO_LARA_OBJECT = 0xbc7300b7,
	AUDIO_LARA_GENERAL_DEATH = 0x192a56d5,
	AUDIO_LARA_KNEES_DEATH = 0xa79e691a,
	AUDIO_LARA_UZI_FIRE = 0x64592a5b,
	AUDIO_LARA_UZI_STOP = 0xeafd4a5a,
	AUDIO_LARA_SHOTGUN = 0x95eb112a,
	AUDIO_LARA_BLOCK_PUSH1 = 0x7fee8f2f,
	AUDIO_LARA_BLOCK_PUSH2 = 0xf850c62d,
	AUDIO_LARA_EMPTY = 0xd675c666,
	AUDIO_LARA_SHOTGUN_SHELL = 0x8bf10a52,
	AUDIO_LARA_BULLETHIT = 0xf88d5041,
	AUDIO_LARA_BLKPULL = 0x8e280c59,
	AUDIO_LARA_FLOATING = 0x582c1c86,
	AUDIO_LARA_FALLDETH = 0xa7bdf45b,
	AUDIO_LARA_GRABHAND = 0x9922a28,
	AUDIO_LARA_GRABBODY = 0x6bf8530,
	AUDIO_LARA_GRABFEET = 0x8e0b3f1e,
	AUDIO_LARA_SWITCHUP = 0x6f1f65b9,
	AUDIO_METEOR = 0x918fdf05,
	AUDIO_WATER_LOOP = 0x93fcbed4,
	AUDIO_UNDERWATER = 0x1e41b5a7,
	AUDIO_UNDERWATER_SWITCH = 0x4ec66295,
	AUDIO_LARA_PICKUP = 0xff5b05a6,
	AUDIO_BLOCK_SOUND = 0x7eb8485f,
	AUDIO_DOOR = 0xf2e65fd6,
	AUDIO_HELICOPTER_LOOP = 0xd27a6ba3,
	AUDIO_ROCK_FALL_CRUMBLE = 0x4d11d08b,
	AUDIO_ROCK_FALL_LAND = 0x311d1730,
	AUDIO_JET_FLY_BY = 0xfa5c88e9,
	AUDIO_STALEGTITE = 0x7da93a9a,
	AUDIO_LARA_THUD = 0xf80de3d4,
	AUDIO_GENERIC_SWOOSH = 0xfd76df2,
	AUDIO_OIL_SMG_FIRE = 0xb29cf4fe,
	AUDIO_CITY_PORTCULLIS = 0xeabcab18,
	AUDIO_SWINGING_FLAMES = 0x4c5d4f19,
	AUDIO_SPINING_HOOKS = 0x538973d2,
	AUDIO_BLAST_CIRCLE = 0xb567d5c1,
	AUDIO_BAZOOKA_FIRE = 0xef6367c9,
	AUDIO_HECKLER_KOCH_FIRE = 0xd4a7bedb,
	AUDIO_WATERFALL_LOOP = 0x7673017c,
	AUDIO_CROC_ATTACK = 0xb548cb2,
	AUDIO_CROC_DEATH = 0x519eba5b,
	AUDIO_PORTCULLIS_UP = 0x7af565f2,
	AUDIO_PORTCULLIS_DOWN = 0xb5170468,
	AUDIO_T_REX_ATTACK = 0x21ccd40d,
	AUDIO_BODY_SLAM = 0x3c67bbf5,
	AUDIO_POWER_HUM_LOOP = 0x369d7d8c,
	AUDIO_T_REX_ROAR = 0xd1150d30,
	AUDIO_T_REX_FOOTSTOMP = 0x1fd7aba,
	AUDIO_T_REX_SNIFF = 0x3c86029a,
	AUDIO_ARMY_SMG_FIRE = 0x66009ecb,
	AUDIO_ARMY_SMG_DEATH = 0x6a1e6f80,
	AUDIO_ARMY_SMG_FOOTSTEPS = 0xdc91f023,
	AUDIO_WING_MUTE_ATTACK = 0xe3918f6a,
	AUDIO_WING_MUTE_DEATH = 0xc0de8966,
	AUDIO_WING_MUTE_FLYING = 0xf579aa5e,
	AUDIO_RAT_ATTACK = 0x788d1ec5,
	AUDIO_RAT_DEATH = 0x70923261,
	AUDIO_TIGER_ROAR = 0x645b6838,
	AUDIO_TIGER_BITE = 0x320f2a03,
	AUDIO_TIGER_STRIKE = 0x668d68ba,
	AUDIO_TIGER_DEATH = 0x5591ab69,
	AUDIO_TIGER_GROWL = 0xba3ccf89,
	AUDIO_HECKLER_KOCH_STOP = 0xc3444c82,
	AUDIO_EXPLOSION1 = 0x9e786a10,
	AUDIO_EXPLOSION2 = 0xadd10bed,
	AUDIO_EARTHQUAKE_LOOP = 0xd3911004,
	AUDIO_MENU_ROTATE = 0x17ced7c,
	AUDIO_MENU_CHOOSE = 0xc31d443d,
	AUDIO_MENU_GAMEBOY = 0x6989d1c0,
	AUDIO_MENU_SPININ = 0x8e77bc62,
	AUDIO_MENU_SPINOUT = 0x9250ae39,
	AUDIO_MENU_STOPWATCH = 0xbbc6e6b8,
	AUDIO_MENU_GUNS = 0xc5005d8,
	AUDIO_MENU_PASSPORT = 0xae9733f1,
	AUDIO_MENU_MEDI = 0x488b12a0,
	AUDIO_LARA_CLIMB_WALLS_NOISE = 0x23d466f6,
	AUDIO_VERY_LIGHT_WATER = 0xef891d6e,
	AUDIO_TARGET_HITS = 0x5de983e2,
	AUDIO_TARGET_SMASH = 0x779159cd,
	AUDIO_DESSERT_EAGLE_FIRE = 0x7301dc1d,
	AUDIO_LARA_MINI_LOAD = 0xc2e2bab1,
	AUDIO_LARA_MINI_LOCK = 0x6cc9dac2,
	AUDIO_LARA_MINI_FIRE = 0xc645eb23,
	AUDIO_GATE_OPENING = 0xb5aad8cf,
	AUDIO_LARA_ELECTRIC_LOOP = 0x3ad3e7f7,
	AUDIO_LARA_ELECTRIC_CRACKLES = 0x13965838,
	AUDIO_COMMANDER = 0x7c0f3fe9,
	AUDIO_SWITCH_COVER = 0xbd3bc9bb,
	AUDIO_CLEANER_FUSEBOX = 0x54770d7e,
	AUDIO_CROW_CAW = 0x5dde8490,
	AUDIO_CROW_WING_FLAP = 0x3036a1b1,
	AUDIO_CROW_DEATH = 0xf67a02af,
	AUDIO_CROW_ATTACK = 0x3acda31d,
	AUDIO_SOFT_WIND_LOOP = 0xa318da40,
	AUDIO_SWAT_SMG_FIRE = 0x85c02b1f,
	AUDIO_LIZARD_MAN_ATTACK_1 = 0x45fb1f7c,
	AUDIO_LIZARD_MAN_ATTACK_2 = 0x36fa00a6,
	AUDIO_LIZARD_MAN_DEATH = 0x6f20f61,
	AUDIO_LIZARD_MAN_CLIMB = 0xd61e19eb,
	AUDIO_LIZARD_MAN_FIRE = 0xe441a8c3,
	AUDIO_GENERIC_BODY_SLAM = 0x92ea2934,
	AUDIO_HECKER_KOCH_OVERLAY = 0xa94538b5,
	AUDIO_LARA_SPIKE_DEATH = 0xa2301f51,
	AUDIO_LARA_DEATH3 = 0xa0f03efa,
	AUDIO_ROLLING_BALL = 0x1b075146,
	AUDIO_TUBE_LOOP = 0x9db412ad,
	AUDIO_RUMBLE_NEXTDOOR = 0x4d775b,
	AUDIO_LOOP_FOR_SMALL_FIRES = 0x8d785ea6,
	AUDIO_DART_GUN = 0x224a98bb,
	AUDIO_QUAD_START = 0xb0eccbbe,
	AUDIO_QUAD_IDLE = 0x86333208,
	AUDIO_QUAD_ACCELERATE = 0xceded011,
	AUDIO_QUAD_MOVE = 0x7557c444,
	AUDIO_QUAD_STOP = 0x35d91c53,
	AUDIO_BATS_1 = 0x29bedbf2,
	AUDIO_LOOP_FOR_GAS_HISS = 0x57c9961f,
	AUDIO_LAUNCHER_1 = 0x4d91a16f,
	AUDIO_LAUNCHER_2 = 0x4f47a9af,
	AUDIO_TRAPDOOR_OPEN = 0xeb65a2a8,
	AUDIO_TRAPDOOR_CLOSE = 0x650d6adb,
	AUDIO_RESERVOIR_FLUSH = 0x985ea3a0,
	AUDIO_MACAQUE_SATND_WAIT = 0xf115a587,
	AUDIO_MACAQUE_ATTACK_LOW = 0xe4dfceff,
	AUDIO_MACAQUE_ATTACK_JUMP = 0xaa05dd74,
	AUDIO_MACAQUE_JUMP = 0x4154de40,
	AUDIO_MACAQUE_DEATH = 0xdd522ba7,
	AUDIO_SEAL_MUTE_FIRE = 0x963b8ce,
	AUDIO_SEAL_MUTE_BREATH_IN = 0x95ea3410,
	AUDIO_SEAL_MUTE_FIRE_2 = 0x5880ece7,
	AUDIO_SEAL_MUTE_FOOT = 0xaceeb343,
	AUDIO_SEAL_MUTE_DEATH = 0xd427cb67,
	AUDIO_SEAL_MUTE_BRUSH_TAIL = 0x89ba0f2e,
	AUDIO_SEAL_MUTE_HIT_FLR = 0xf249b598,
	AUDIO_DOG_ATTACK_1 = 0x45a5f0b5,
	AUDIO_DOG_AWARE = 0x20f1bfd7,
	AUDIO_DOG_FOOT_1 = 0xd306ee6f,
	AUDIO_DOG_JUMP = 0xcbb44ee1,
	AUDIO_DOG_GROWL = 0xb7d3c98c,
	AUDIO_DOG_DEATH = 0xac79643a,
	AUDIO_VULTURE_WING_FLAP = 0x9068c88c,
	AUDIO_VULTURE_ATTACK = 0x705ef765,
	AUDIO_VULTURE_DIE = 0x56bbfcc1,
	AUDIO_VULTURE_GLIDE = 0x5ea5b40d,
	AUDIO_SCUBA_DIVER_FLIPPER = 0x3a0c467b,
	AUDIO_SCUBA_DIVER_ARM = 0xe98e49b,
	AUDIO_SCUBA_DIVER_BREATH_W = 0x152e7dd9,
	AUDIO_SCUBA_DIVER_BREATH_S = 0x68231edd,
	AUDIO_LONDON_MERCENARY_DEATH = 0x4144253c,
	AUDIO_CLEANER_LOOP = 0x500e733a,
	AUDIO_SCUBA_DIVER_DEATH = 0x6754276c,
	AUDIO_SCUBA_DIVER_DIVING = 0x29a87e65,
	AUDIO_BOAT_START = 0x573df140,
	AUDIO_BOAT_IDLE = 0xfcffc71c,
	AUDIO_BOAT_ACCELERATE = 0x69875a73,
	AUDIO_BOAT_MOVING = 0x7b8b21a7,
	AUDIO_BOAT_STOP = 0xab2e5c92,
	AUDIO_BOAT_SLOW_DOWN = 0x23264890,
	AUDIO_JET_ROOFS = 0xe2c77338,
	AUDIO_QUAD_SIDE_IMPACT = 0x9801cb20,
	AUDIO_QUAD_FRONT_IMPACT = 0x5b8e0dd3,
	AUDIO_QUAD_LAND = 0x2e71e2af,
	AUDIO_FLAME_THROWER_LOOP = 0xda8e69e6,
	AUDIO_RUMMBLE = 0x2d637047,
	AUDIO_DRILL_BIT_1 = 0x661025bf,
	AUDIO_VERY_SMALL_WINCH = 0xc26bf1,
	AUDIO_ALARM_1 = 0xe83b6e66,
	AUDIO_MINE_CART_TRACK_LOOP = 0x997dcd7,
	AUDIO_MINE_CART_PULLY_LOOP = 0x5942a6f7,
	AUDIO_MINE_CART_CLUNK_START = 0x9286a011,
	AUDIO_SAVE_CRYSTAL = 0xbfb52966,
	AUDIO_WOOD_GATE = 0x3351671f,
	AUDIO_METAL_SHUTTERS_SMASH = 0x79c23b75,
	AUDIO_UNDERWATER_FAN_ON = 0xa53165d0,
	AUDIO_UNDERWATER_FAN_STOP = 0xf4a6432a,
	AUDIO_SMALL_FAN_ON = 0xd3658402,
	AUDIO_SWINGING_BOX_BAG = 0xf4fac2d6,
	AUDIO_MINE_CART_SREECH_BRAKE = 0x726aeeef,
	AUDIO_SPANNER = 0x5db1a193,
	AUDIO_SMALL_METAL_SHUTTERS = 0x393c4991,
	AUDIO_AREA51_SWINGER_START = 0xcc8d88c1,
	AUDIO_AREA51_SWINGER_STOP = 0xede67444,
	AUDIO_AREA51_SWINGER_LOOP = 0x9e2303ff,
	AUDIO_SLIDE_DOOR_CLOSE_1 = 0xa35f34ec,
	AUDIO_SLIDE_DOOR_CLOSE_2 = 0x97b41c76,
	AUDIO_OILDRUM_ROLL = 0xeed16a6b,
	AUDIO_SIDE_BLADE_SWING = 0x2b5b8530,
	AUDIO_SIDE_BLADE_BACK = 0x8013cb67,
	AUDIO_SKEL_TRAP_PART_1 = 0xcf65d537,
	AUDIO_SKEL_TRAP_PART_2 = 0xe2d8fabd,
	AUDIO_SMALL_FAN = 0x3b2ecdd0,
	AUDIO_TONY_BOSS_STONE_DEATH = 0x2c815252,
	AUDIO_TONY_BOSS_NORMAL_DEATH = 0xee9df44c,
	AUDIO_TONY_BOSS_LAUGH = 0xd25c22f0,
	AUDIO_LONDON_BOSS_SHOOTER = 0x2b07d299,
	AUDIO_HARD_WIND_LOOP = 0xfbcf4094,
	AUDIO_COMPY_ATTACK = 0x6c5517f6,
	AUDIO_COMPY_JUMP = 0xd745ec76,
	AUDIO_COMPY_WAIT = 0x1979cfa6,
	AUDIO_COMPY_DIE = 0x25b22ab5,
	AUDIO_COMPY_RUN_WALK = 0x979af30e,
	AUDIO_BLOWPIPE_NATIVE_FEET = 0x5d12e96c,
	AUDIO_BLOWPIPE_NATIVE_ATTACK = 0xc782316,
	AUDIO_BLOWPIPE_NATIVE_DEATH = 0xbf785b67,
	AUDIO_BLOWPIPE_NATIVE_BLOW = 0x2cafec79,
	AUDIO_BLOWPIPE_NATIVE_SWOOSH = 0x20cb41a1,
	AUDIO_SHIVA_WALK_MURMA = 0x9761d452,
	AUDIO_RAPTOR_FEET = 0x6ebabcac,
	AUDIO_RAPTOR_ATTACK_1 = 0x204668f8,
	AUDIO_RAPTOR_ATTACK_2 = 0x1bfa6140,
	AUDIO_RAPTOR_ATTACK_3 = 0x61c81fbb,
	AUDIO_RAPTOR_ROAR = 0x42297f06,
	AUDIO_RAPTOR_DIE_1 = 0xb8d1a2e,
	AUDIO_RAPTOR_DIE_2 = 0xc129a55,
	AUDIO_HUGE_ROCKET_LOOP = 0x9efb15b5,
	AUDIO_SHIVA_SWORD_1 = 0x2e5f04ce,
	AUDIO_SHIVA_SWORD_2 = 0x1ca2815,
	AUDIO_SHIVA_DEATH = 0x472deaab,
	AUDIO_SHIVA_FOOTSTEP = 0x25d131d5,
	AUDIO_SHIVA_LAUGH = 0xa1026d8,
	AUDIO_SHIVA_HIT_GROUND = 0x4a807da,
	AUDIO_HYBRID_FOOT = 0x1b8e42df,
	AUDIO_HYBRID_HOOF = 0x26538c6d,
	AUDIO_HYBRID_ATTACK = 0x69ae66ef,
	AUDIO_HYBRID_DEATH = 0x3b0bb6b8,
	AUDIO_HYBRID_SWOOSH = 0x8522ccf1,
	AUDIO_SMALL_SWITCH = 0x6a17aa67,
	AUDIO_CLAW_MUTE_FOOTSTEPS = 0xe550e424,
	AUDIO_CLAW_MUTE_ATTACK = 0x93ae4126,
	AUDIO_CLAW_MUTE_DEATH = 0x56dc5d44,
	AUDIO_CLAW_MUTE_BODY_THUD = 0x9d08067,
	AUDIO_CLAW_MUTE_LAZER = 0xf7f020ac,
	AUDIO_CLAW_MUTE_SWOOSH = 0x3d056820,
	AUDIO_CLAW_MUTE_CLAW = 0x791b345d,
	AUDIO_HYBRID_BODY_SLAM = 0x545c50e8,
	AUDIO_SMALL_DOOR_SUBWAY = 0xc2ae7c8e,
	AUDIO_DEATH_SLIDE_GRAB = 0xa016b44,
	AUDIO_DEATH_SLIDE_GO = 0x3111a41,
	AUDIO_DEATH_SLIDE_STOP = 0x577835db,
	AUDIO_RADAR_BLIP = 0x3f100545,
	AUDIO_BOB_FEET = 0x447549db,
	AUDIO_BOB_ATTACK = 0x72a73ee8,
	AUDIO_BOB_DEATH = 0x802784ba,
	AUDIO_BOB_CLIMB = 0x863e975e,
	AUDIO_BOB_GET_DOWN = 0xa45fe568,
	AUDIO_FOOTSTEPS_MUD = 0x10197891,
	AUDIO_FOOTSTEPS_ICE = 0x676e2d78,
	AUDIO_FOOTSTEPS_GRAVEL = 0xdb102b5e,
	AUDIO_FOOTSTEPS_SAND___GRASS = 0x8689483f,
	AUDIO_FOOTSTEPS_WOOD = 0x54c62bb6,
	AUDIO_FOOTSTEPS_SNOW = 0xa555df1b,
	AUDIO_FOOTSTEPS_METAL = 0xabd16350,
	AUDIO_LOOP_FOR_LONDON = 0xa237fad7,
	AUDIO__1ST_LOOP_FOR_BIG_DRILL = 0x406016f2,
	AUDIO_SMALL_DOOR_SUBWAY_CLOSE = 0xc50d5b0e,
	AUDIO__2ND_LOOP_FOR_BIG_DRILL = 0x6ee18ece,
	AUDIO_ENGLISH_HOY = 0x8c583b1,
	AUDIO_AMERCAN_HOY = 0x1128e3e5,
	AUDIO_OIL_RED_SMG_DEATH = 0xe99a2253,
	AUDIO_RADIO_LOOP = 0x1a79de5d,
	AUDIO_PUNK_ATTACK = 0x4259e469,
	AUDIO_PUNK_DEATH = 0xc5ce1d1c,
	AUDIO_SECURITY_GUARD_FIRE = 0xf20afad7,
	AUDIO_SECURITY_GUARD_DEATH = 0xa99d479d,
	AUDIO_LAZER_LOOP = 0x72e48b61,
	AUDIO_WINSTON_BRUSH_OFF = 0xb8795620,
	AUDIO_WINSTON_CUPS = 0xd988d566,
	AUDIO_WINSTON_HU = 0x700c72f9,
	AUDIO_WINSTON_BULLET_TRAY = 0xaed5632a,
	AUDIO_WINSTON_FOOTSTEPS = 0x32bf993a,
	AUDIO_WINSTON_TAKE_HIT = 0xe06d7a66,
	AUDIO_WINSTON_GET_UP = 0xf9cb164c,
	AUDIO_WINSTON_FART = 0x19a448e7,
	AUDIO_WALL_BLADES = 0x5356c1d0,
	AUDIO_MACAQUE_CHATTER = 0x66912839,
	AUDIO_MACAQUE_ROLL = 0x80ec6987,
	AUDIO_WHALE_CALL = 0xdde023f9,
	AUDIO_GENERATOR_LOOP = 0x268d99ea,
	AUDIO_GENERATOR_SHITTING = 0x9d04f1a3,
	AUDIO_GASMETER = 0x4846e920,
	AUDIO_LARA_TURN_WHEEL = 0x91139704,
	AUDIO_COBRA_HISS = 0x7774c1f7,
	AUDIO_DART_SPITT = 0x6732a69b,
	AUDIO_RATTLE_SNAKE = 0xc5de8f6e,
	AUDIO_SWING_PUMP = 0x117ad495,
	AUDIO_SQEEK = 0xe22983be,
	AUDIO_DRIPS_REVERB = 0xa33437e3,
	AUDIO_TONK = 0xdd85f29b,
	AUDIO_BOO_MUTE = 0x5379df8e,
	AUDIO_VENDING_MACHINE_LOOP = 0x65e994bb,
	AUDIO_VENDING_SPIT = 0xc3ce7c0a,
	AUDIO_DOORBELL = 0x559d52e0,
	AUDIO_BURGLAR_ALARM = 0x25fccac8,
	AUDIO_BOAT_SCRAPE = 0x91b6d81f,
	AUDIO_TICK_TOCK = 0x129590eb,
	AUDIO_WILARD_FOOT_STEPS = 0xb16b4e05,
	AUDIO_WILARD_ATTACK = 0xae388913,
	AUDIO_WILARD_TAKE_HIT = 0xb4fb3817,
	AUDIO_WILARD_LEGS_SHUFFLE = 0xad3ab4d1,
	AUDIO_WILARD_FIRE_CHARGE = 0xf1876047,
	AUDIO_WILARD_FIRE_SHOOT = 0x7849d9e8,
	AUDIO_WILARD_ODD_NOISE = 0xb354a263,
	AUDIO_WILARD_STAB = 0x4a92081,
	AUDIO_LITTLE_SUB_LOOP = 0x7e5c1379,
	AUDIO_LITTLE_SUB_START = 0x70eed36b,
	AUDIO_LITTLE_SUB_STOP = 0x91aee0ba,
	AUDIO_LONDON_BOSS_DIE_PART_1 = 0xfe44a4f0,
	AUDIO_LONDON_BOSS_DIE_PART_2 = 0xe7337612,
	AUDIO_LONDON_BOSS_FIRE = 0xa8036e6e,
	AUDIO_LONDON_BOSS_SUMMON = 0x3eeea805,
	AUDIO_LONDON_BOSS_TAKE_HIT = 0xf4acaad4,
	AUDIO_LONDON_BOSS_VAULT = 0xba72a882,
	AUDIO_LONDON_BOSS_SUMMON_NOT = 0xa9fcc997,
	AUDIO_LONDON_BOSS_LAUGH = 0x1f2d12e7,
	AUDIO_WATER_MILL = 0xf5e0400,
	AUDIO_PLUG_WINCH = 0x9d2cee44,
	AUDIO_GIANT_METAL_WHEELS = 0x5db3a6cf,
	AUDIO_TRIBOSS_ATTACK = 0x7d6250d4,
	AUDIO_TRIBOSS_TAKE_HIT = 0x9752499b,
	AUDIO_TRIBOSS_TURN_CHAIR = 0x7e2bcdb,
	AUDIO_TRIBOSS_SHOOT = 0x78753fc,
	AUDIO_TRIBOSS_DEATH_VOCAL = 0x9f2d4dc3,
	AUDIO_TRIBOSS_CHAIR_2 = 0x3cc2b2d6,
	AUDIO_TONY_BOSS_SHOOT_1 = 0x2729220a,
	AUDIO_TONY_BOSS_SHOOT_2 = 0x4bee6cb4,
	AUDIO_TONY_BOSS_SHOOT_3 = 0xa7cf2594,
	AUDIO_TONY_BOSS_ATTACK = 0x5359bb52,
};
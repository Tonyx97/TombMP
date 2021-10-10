#pragma once

#define MAXIMUM_LEVELS 24

enum title_options
{
	START_GAME = 0,
	EXIT_GAME = (1 << 0),
	LEVEL_COMPLETED = (1 << 1),
	LEVEL_CHANGE = (1 << 2),
	OPENING_GAME = (1 << 3)
};

enum engine_level
{
	LV_GYM = 0,
	LV_FIRSTLEVEL,
	LV_JUNGLE = 1,
	LV_TEMPLE,
	LV_QUADBIKE,
	LV_INDIABOSS,
	LV_SHORE,
	LV_CRASH,
	LV_RAPIDS,
	LV_PACBOSS,
	LV_ROOFTOPS,
	LV_SEWER,
	LV_TOWER,
	LV_OFFICE,
	LV_DESERT,
	LV_COMPOUND,
	LV_AREA51,
	LV_ANTARC,
	LV_MINES,
	LV_CITY,
	LV_CHAMBER,
	LV_STPAULS
};

enum add_inv_types
{
	ADDINV_PISTOLS,
	ADDINV_SHOTGUN,
	ADDINV_AUTOPISTOLS,
	ADDINV_UZIS,
	ADDINV_HARPOON,
	ADDINV_M16,
	ADDINV_ROCKET,
	ADDINV_GRENADE,

	ADDINV_PISTOL_AMMO,
	ADDINV_SHOTGUN_AMMO,
	ADDINV_AUTOPISTOLS_AMMO,
	ADDINV_UZI_AMMO,
	ADDINV_HARPOON_AMMO,
	ADDINV_M16_AMMO,
	ADDINV_ROCKET_AMMO,
	ADDINV_GRENADE_AMMO,
	ADDINV_FLARES,

	ADDINV_MEDI,
	ADDINV_BIGMEDI,

	ADDINV_PICKUP1,
	ADDINV_PICKUP2,
	ADDINV_PUZZLE1,
	ADDINV_PUZZLE2,
	ADDINV_PUZZLE3,
	ADDINV_PUZZLE4,
	ADDINV_KEY1,
	ADDINV_KEY2,
	ADDINV_KEY3,
	ADDINV_KEY4,
	ADDINV_SAVEGAME_CRYSTAL,
	ADDINV_NUMBEROF
};

enum gf_event_types
{
	GFE_PICTURE = 0,
	GFE_LIST_START,
	GFE_LIST_END,
	GFE_PLAYFMV,
	GFE_STARTLEVEL,
	GFE_CUTSCENE,
	GFE_LEVCOMPLETE,
	GFE_DEMOPLAY,
	GFE_JUMPTO_SEQ,
	GFE_END_SEQ,
	GFE_SETTRACK,
	GFE_SUNSET,
	GFE_LOADINGPIC,
	GFE_DEADLY_WATER,
	GFE_REMOVE_WEAPONS,
	GFE_GAMECOMPLETE,
	GFE_CUTANGLE,
	GFE_NOFLOOR,
	GFE_ADD2INV,
	GFE_STARTANIM,
	GFE_NUMSECRETS,
	GFE_KILL2COMPLETE,
	GFE_REMOVE_AMMO
};

enum game_string_ids
{
	GT_MAIN_HEADING = 0,
	GT_OPTION_HEADING,
	GT_KEYS_HEADING,
	GT_GAMEOVER_HEADING,
	// Passport Text Section
	GT_LOADGAME,
	GT_SAVEGAME,
	GT_STARTGAME,
	GT_RESTARTLEVEL,
	GT_EXIT2TITLE,
	GT_EXITDEMO,
	GT_EXITGAME,
	GT_SELECTLEVEL,
	GT_SAVEPOSITION,
	// Detail text
	GT_DETAIL,
	GT_HIGH_DETAIL,
	GT_MEDIUM_DETAIL,
	GT_LOW_DETAIL,
	// Control strings
	GT_WALK,
	GT_ROLL,
	GT_RUN,
	GT_LEFT,
	GT_RIGHT,
	GT_BACK,
	GT_STEPLEFT1,
	GT_STEPLEFT2,
	GT_STEPRIGHT1,
	GT_STEPRIGHT2,
	GT_LOOK,
	GT_JUMP,
	GT_ACTION,
	GT_DRAWWEAPON1,
	GT_DRAWWEAPON2,
	GT_INVENTORY,
	GT_USEFLARE,
	GT_STEPSHIFT,
	// Inventory Item Strings
	GT_STOPWATCH,
	GT_PISTOLS,
	GT_SHOTGUN,
	GT_AUTOPISTOLS,
	GT_UZIS,
	GT_HARPOON,
	GT_M16,
	GT_ROCKETLAUNCHER,
	GT_GRENADELAUNCHER,
	GT_FLARE,

	GT_PISTOLCLIPS,
	GT_SHOTGUNSHELLS,
	GT_AUTOPISTOLCLIPS,
	GT_UZICLIPS,
	GT_HARPOONBOLTS,
	GT_M16CLIPS,
	GT_ROCKETS,
	GT_GRENADES,

	GT_SMALLMEDI,
	GT_LARGEMEDI,
	GT_PICKUP,
	GT_PUZZLE,
	GT_KEY,
	GT_GAME,

	GT_GYM,
	GT_LOADING,

	// Stats text
	GT_STAT_TIME,
	GT_STAT_SECRETS,
	GT_STAT_LOCATION,
	GT_STAT_KILLS,
	GT_STAT_AMMO,
	GT_STAT_RATIO,
	GT_STAT_SAVES,
	GT_STAT_DISTANCE,
	GT_STAT_HEALTH,

	GT_SECURITY_TAG,

	GT_NONE,
	GT_FINISH,
	GT_BESTTIMES,
	GT_NOTIMES,
	GT_NOTAVAILABLE,
	GT_CURRENTPOS,
	GT_GAMESTATS,
	GT_OF,
	GT_STORY_SO_FAR,
	GT_ICON1,
	GT_ICON2,
	GT_ICON3,
	GT_ICON4,
	GT_CRYSTAL,
	GT_LSLONDON,
	GT_LSNEVADA,
	GT_LSSPAC,
	GT_LSANTARC,
	GT_LSPERU,
	GT_LEVELSELECT,
	GT_SPACE,
	GT_NUM_GAMESTRINGS
};

enum pc_string_ids
{
	PCSTR_DETAILLEVEL = 0,
	PCSTR_DEMOMODE,
	PCSTR_SOUND,
	PCSTR_CONTROLS,
	PCSTR_GAMMA,
	PCSTR_SETVOLUME,
	PCSTR_USERKEYS,
	// Savegame Strings
	PCSTR_SAVEMESS1,
	PCSTR_SAVEMESS2,
	PCSTR_SAVEMESS3,
	PCSTR_SAVEMESS4,
	PCSTR_SAVEMESS5,
	PCSTR_SAVEMESS6,
	PCSTR_SAVEMESS7,
	PCSTR_SAVEMESS8,
	PCSTR_SAVESLOT,
	PCSTR_OFF,
	PCSTR_ON,
	PCSTR_SETUPSOUND,
	PCSTR_DEFAULTKEYS,
	PCSTR_DOZY_STRING,
	PCSTR_VIDEOTITLE,
	PCSTR_RESOLUTION,
	PCSTR_ZBUFFER,
	PCSTR_FILTERING,
	PCSTR_DITHER,
	PCSTR_TRUEALPHA,
	PCSTR_SKY,
	PCSTR_SPARE8,
	PCSTR_SPARE9,
	PCSTR_SPARE10,
	PCSTR_SPARE11,
	PCSTR_SPARE12,
	PCSTR_SPARE13,
	PCSTR_SPARE14,
	PCSTR_SPARE15,
	PCSTR_SPARE16,
	PCSTR_SPARE17,
	PCSTR_SPARE18,
	PCSTR_SPARE19,
	PCSTR_SPARE20,
	PCSTR_NUM_STRINGS
};

struct GAMEFLOW_INFO
{
	// 16 x int32_t  (16)
	int32_t firstOption;					// First option to do (usually EXIT_TO_TITLE)
	int32_t title_replace;				// Replace the title with START_GAME|LEVEL1 / STARTDEMO|LEVEL1 / ETC (unused if title_enabled==1)
	int32_t ondeath_demo_mode;			// What to do on death in demo mode EXIT_TO_TITLE / STARTDEMO
	int32_t ondeath_ingame;				// What to do on death in a level EXIT_TO_TITLE
	int32_t noinput_time;				// Used on title page (and in game/demos if noinput_timeout==1)
	int32_t on_demo_interrupt;
	int32_t on_demo_end;
	int32_t unused1[9];

	// 24 x int16_t	(12)
	int16_t num_levels;					// Game levels total inc GYM
	int16_t num_picfiles;				// Pics
	int16_t num_titlefiles;				// Title files!
	int16_t num_fmvfiles;
	int16_t num_cutfiles;
	int16_t num_demos;
	int16_t title_track;
	int16_t singlelevel;					// Single playable level number for demos
	uint16_t unused2[16];

	// 4 x uint16 for bits	(2)
	uint16_t demoversion : 1;
	uint16_t title_disabled : 1;			// Disable title page (used for demos)
	uint16_t cheatmodecheck_disabled : 1;	// Retail version of cheat
	uint16_t noinput_timeout : 1;			// Timeout if no input!
	uint16_t loadsave_disabled : 1;
	uint16_t screensizing_disabled : 1;
	uint16_t lockout_optionring : 1;		// Lock out main ring
	uint16_t dozy_cheat_enabled : 1;		// Allow simple cheat input
	uint16_t cyphered_strings : 1;
	uint16_t gym_enabled : 1;
	uint16_t play_any_level : 1;			// Requester on middle pages of passport
	uint16_t cheat_enable : 1;			// 'C' key fly, get weapons cheat
	uint16_t securitytag : 1;
	uint16_t unused3[3];

	// 8 x char	(2)
	uint8_t cypher_code;
	uint8_t language;
	uint8_t secret_track;
	uint8_t stats_track;
	char pads[4];
};

inline GAMEFLOW_INFO gameflow {};

extern char GF_Description[];
extern char** GF_Level_Names;
extern char** GF_GameStrings;
extern char** GF_PCStrings;

extern char** GF_Puzzle1Strings;
extern char** GF_Puzzle2Strings;
extern char** GF_Puzzle3Strings;
extern char** GF_Puzzle4Strings;
extern char** GF_Secret1Strings;
extern char** GF_Secret2Strings;
extern char** GF_Secret3Strings;
extern char** GF_Secret4Strings;
extern char** GF_Special1Strings;
extern char** GF_Special2Strings;
extern char** GF_Pickup1Strings;
extern char** GF_Pickup2Strings;
extern char** GF_Key1Strings;
extern char** GF_Key2Strings;
extern char** GF_Key3Strings;
extern char** GF_Key4Strings;
extern const char* GF_inv_types[];

extern uint32_t GF_ScriptVersion;
extern int16_t GF_SunsetEnabled;
extern int16_t GF_NoFloor;
extern int16_t GF_NumSecrets;
extern char** GF_picfilenames;
extern char** GF_titlefilenames;
extern char** GF_fmvfilenames;
extern char** GF_levelfilenames;
extern char** GF_cutscenefilenames;
extern char** GF_demofilenames;

extern char* GF_PCStrings_buffer;

extern char* GF_levelnames_buffer;
extern char* GF_GameStrings_buffer;
extern char* GF_Puzzle1Strings_buffer;
extern char* GF_Puzzle2Strings_buffer;
extern char* GF_Puzzle3Strings_buffer;
extern char* GF_Puzzle4Strings_buffer;
extern char* GF_Secret1Strings_buffer;
extern char* GF_Secret2Strings_buffer;
extern char* GF_Secret3Strings_buffer;
extern char* GF_Secret4Strings_buffer;
extern char* GF_Special1Strings_buffer;
extern char* GF_Special2Strings_buffer;
extern char* GF_Pickup1Strings_buffer;
extern char* GF_Pickup2Strings_buffer;
extern char* GF_Key1Strings_buffer;
extern char* GF_Key2Strings_buffer;
extern char* GF_Key3Strings_buffer;
extern char* GF_Key4Strings_buffer;

extern char* GF_picfilenames_buffer;
extern char* GF_titlefilenames_buffer;
extern char* GF_fmvfilenames_buffer;
extern char* GF_levelfilenames_buffer;
extern char* GF_cutscenefilenames_buffer;
extern char* GF_demofilenames_buffer;

extern int16_t* GF_sequence_buffer;

extern int16_t* GF_level_sequence_list[];
extern char	GF_secret_totals[];

void GF_ModifyInventory();
int GF_LoadScriptFile(const char* fname);
int GF_InterpretSequence(int16_t* ptr);
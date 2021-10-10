#include <specific/stypes.h>

#include "gameflow.h"

char GF_Description[256] { 0 };

int16_t* GF_level_sequence_list[MAXIMUM_LEVELS];

char* GF_levelnames_buffer;
char* GF_GameStrings_buffer;
char* GF_PCStrings_buffer;
char* GF_Puzzle1Strings_buffer;
char* GF_Puzzle2Strings_buffer;
char* GF_Puzzle3Strings_buffer;
char* GF_Puzzle4Strings_buffer;
char* GF_Secret1Strings_buffer;
char* GF_Secret2Strings_buffer;
char* GF_Secret3Strings_buffer;
char* GF_Secret4Strings_buffer;
char* GF_Special1Strings_buffer;
char* GF_Special2Strings_buffer;
char* GF_Pickup1Strings_buffer;
char* GF_Pickup2Strings_buffer;
char* GF_Key1Strings_buffer;
char* GF_Key2Strings_buffer;
char* GF_Key3Strings_buffer;
char* GF_Key4Strings_buffer;

char* GF_picfilenames_buffer;
char* GF_titlefilenames_buffer;
char* GF_fmvfilenames_buffer;
char* GF_levelfilenames_buffer;
char* GF_cutscenefilenames_buffer;
char* GF_demofilenames_buffer;
int16_t* GF_sequence_buffer;

char** GF_Level_Names;
char** GF_GameStrings;
char** GF_PCStrings;
char** GF_Puzzle1Strings;
char** GF_Puzzle2Strings;
char** GF_Puzzle3Strings;
char** GF_Puzzle4Strings;
char** GF_Secret1Strings;
char** GF_Secret2Strings;
char** GF_Secret3Strings;
char** GF_Secret4Strings;
char** GF_Special1Strings;
char** GF_Special2Strings;
char** GF_Pickup1Strings;
char** GF_Pickup2Strings;
char** GF_Key1Strings;
char** GF_Key2Strings;
char** GF_Key3Strings;
char** GF_Key4Strings;

char** GF_picfilenames;
char** GF_titlefilenames;
char** GF_fmvfilenames;
char** GF_levelfilenames;
char** GF_cutscenefilenames;
char** GF_demofilenames;

const char* GF_inv_types[] =
{
	"PISTOLS",
	"SHOTGUN",
	"AUTOPISTOLS",
	"UZIS",
	"HARPOON",
	"M16",
	"ROCKETLAUNCHER",
	"GRENADELAUNCHER",

	"PISTOL_AMMO",
	"SHOTGUN_AMMO",
	"AUTOPISTOLS_AMMO",
	"UZI_AMMO",
	"HARPOON_AMMO",
	"M16_AMMO",
	"ROCKETLAUNCHER_AMMO",
	"GRENADELAUNCHER_AMMO",
	"FLARES",

	"MEDI",
	"BIGMEDI",

	"PICKUP1",
	"PICKUP2",
	"PUZZLE1",
	"PUZZLE2",
	"PUZZLE3",
	"PUZZLE4",
	"KEY1",
	"KEY2",
	"KEY3",
	"KEY4",

	"CRYSTAL"
};
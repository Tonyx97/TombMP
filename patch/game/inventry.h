#pragma once

#define MAX_MAP_SHAPES	(32 * 32)
#define C_TRANS			(1 << 15)
#define NO_ACTION		0
#define GAME_ACTION		1
#define END_ACTION		2
#define LOAD_ACTION		4
#define SAVE_ACTION		8
#define SOUND_ACTION	16
#define VIDEO_ACTION	32

enum inv_modes
{
	INV_GAME_MODE = 0,
	INV_KEYS_MODE
};

enum shapes
{
	SHAPE_SPRITE = 1,
	SHAPE_LINE,
	SHAPE_BOX,
	SHAPE_FBOX
};

enum inventory_colours
{
	C_BLACK = 0,
	C_GREY,
	C_WHITE,
	C_RED,
	C_ORANGE,
	C_YELLOW,
	C_GREEN1,
	C_GREEN2,
	C_GREEN3,
	C_GREEN4,
	C_GREEN5,
	C_GREEN6,
	C_DARKGREEN,
	C_GREEN,
	C_CYAN,
	C_BLUE,
	C_MAGENTA,
	C_NUMBER_COLOURS
};

using SG_COL = uint16_t;

inline int16_t option_gamma_level = 0;
inline int16_t Inventory_Displaying = 0;
extern int16_t Inventory_Chosen;
extern int32_t Inventory_ExtraData[];
extern int32_t Inventory_Mode;

extern int16_t Compass_Status;		// 1 = selected

inline int32_t idelay = 0;
inline int32_t idcount = 0;
inline int16_t item_data = 0;

extern SG_COL inv_colours[];

int32_t display_inventory(int32_t inventory_mode);
bool init_inventory(int inventory_mode);
#pragma once

enum
{
	IN_FORWARD_ID,
	IN_BACK_ID,
	IN_LEFT_ID,
	IN_RIGHT_ID,
	IN_JUMP_ID,
	IN_DRAW_ID,
	IN_ACTION_ID,
	IN_SLOW_ID,
	IN_OPTION_ID,
	IN_LOOK_ID,
	IN_STEPL_ID,
	IN_STEPR_ID,
	IN_ROLL_ID,
	IN_PAUSE_ID,
	IN_FLARE_ID,
	IN_SELECT_ID,
	IN_DESELECT_ID,
	IN_DUCK_ID,
	IN_DASH_ID,
	IN_C_ID,
	IN_D_ID,
	IN_SMALL_MEDKIT,
	IN_LARGE_MEDKIT,
	IN_MAX_ID,
};

enum
{
	IN_FORWARD		= (1 << 0),
	IN_BACK			= (1 << 1),
	IN_LEFT			= (1 << 2),
	IN_RIGHT		= (1 << 3),
	IN_JUMP			= (1 << 4),
	IN_DRAW			= (1 << 5),
	IN_ACTION		= (1 << 6),
	IN_SLOW			= (1 << 7),
	IN_OPTION		= (1 << 8),
	IN_LOOK			= (1 << 9),
	IN_STEPL		= (1 << 10),
	IN_STEPR		= (1 << 11),
	IN_ROLL			= (1 << 12),
	IN_PAUSE		= (1 << 13),
	IN_FLARE		= (1 << 14),
	IN_SELECT		= (1 << 15),
	IN_DESELECT		= (1 << 16),
	IN_DUCK			= (1 << 17),
	IN_DASH			= (1 << 18),
	IN_C			= (1 << 19),
	IN_D			= (1 << 20),
};

inline int32_t input = 0,
			  inv_input = 0;

void init_key_map();
bool update_input(bool inventory = false);
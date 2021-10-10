#pragma once

#include <shared/game/math.h>

enum entity_type
{
	ENTITY_TYPE_PLAYER,
	ENTITY_TYPE_LEVEL,
	ENTITY_TYPE_UNKNOWN,
};

enum level_entity_type
{
	ENTITY_LEVEL_TYPE_BLOCK,
	ENTITY_LEVEL_TYPE_AI,
	ENTITY_LEVEL_TYPE_INTERACTIVE,
	ENTITY_LEVEL_TYPE_TRAP,
	ENTITY_LEVEL_TYPE_DOOR,
	ENTITY_LEVEL_TYPE_ANIMATING,
	ENTITY_LEVEL_TYPE_SPECIAL_FX,
	ENTITY_LEVEL_TYPE_PICKUP,
	ENTITY_LEVEL_TYPE_VEHICLE,
	ENTITY_LEVEL_TYPE_NONE,
};

enum entity_flags
{
	ENTITY_FLAG_INVISIBLE = (1 << 0),
	ENTITY_FLAG_FLY_CHEAT = (1 << 1),
};

struct subtype_update_info
{
	int_vec3 pos;

	int16_t room,
			hp,
			flags,
			flags0;

	bool new_active;
};
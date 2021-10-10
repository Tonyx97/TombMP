#include <shared/defs.h>

#include <sol/sol.hpp>

#include <game/physics.h>
#include <game/control.h>
#include <game/items.h>

#include "cf_defs.h"

void cf_physics::register_functions(sol::state* vm)
{
	vm->set_function("getGravity",	[&]()		{ return GRAVITY; });
	vm->set_function("setGravity",	[&](int v)	{ GRAVITY = v; });

	vm->set_function("lineOfSight", [&](int x, int y, int z, int x1, int y1, int z1, uint16_t room) -> std::tuple<int, int, int, uint16_t, bool>
	{
		GAME_VECTOR src { x, y, z, int16_t(room) },
					dest { x1, y1, z1 };

		const bool clear = LOS(&src, &dest);

		return { dest.x, dest.y, dest.z, dest.room_number, clear };
	});

	vm->set_function("getObjectInLineOfSight", [&](int x, int y, int z, int x1, int y1, int z1, uint16_t room) -> ITEM_INFO*
	{
		GAME_VECTOR src { x, y, z, int16_t(room) },
					dest { x1, y1, z1 };

		if (auto item_id = ObjectOnLOS(&src, &dest); item_id != NO_ITEM)
			return &items[item_id];

		return nullptr;
	});
}
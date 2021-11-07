#include <shared/defs.h>

#include <sol/sol.hpp>

#include <specific/standard.h>

#include "cf_defs.h"

#include <specific/drawprimitive.h>
#include <specific/hwrender.h>

#include <mp/game/player.h>
#include <mp/game/level.h>

void cf_item::register_functions(sol::state* vm)
{
	vm->set_function("getItemObjectID", [&](ITEM_INFO* item) { return item->object_number; });

	vm->set_function("getItemMeshBits", [&](ITEM_INFO* item)				{ return item->mesh_bits; });
	vm->set_function("setItemMeshBits", [&](ITEM_INFO* item, uint32_t val)  { item->mesh_bits = val; });
	
	vm->set_function("getItemObjectID", [&](ITEM_INFO* item)				{ return item->object_number; });
	vm->set_function("setItemObjectID", [&](ITEM_INFO* item, int16_t val)   { item->object_number = val; });

	vm->set_function("getItemRoom", [&](ITEM_INFO* item)				{ return IsValidItem(item) ? item->room_number : 0; });
	vm->set_function("setItemRoom", [&](ITEM_INFO* item, int16_t val)	{ item->room_number = val; });
	
	vm->set_function("getItemSpeed", [&](ITEM_INFO* item)				{ return item->speed; });
	vm->set_function("setItemSpeed", [&](ITEM_INFO* item, int16_t val)  { item->speed = val; });

	vm->set_function("getItemFallspeed", [&](ITEM_INFO* item)				{ return item->fallspeed; });
	vm->set_function("setItemFallspeed", [&](ITEM_INFO* item, int16_t val)  { item->fallspeed = val; });

	vm->set_function("getItemHealth", [&](ITEM_INFO* item)				{ return item->hit_points; });
	vm->set_function("setItemHealth", [&](ITEM_INFO* item, int16_t val) { item->hit_points = val; });

	vm->set_function("getItemCarriedItem", [&](ITEM_INFO* item)				 { return item->carried_item; });
	vm->set_function("setItemCarriedItem", [&](ITEM_INFO* item, int16_t val) { item->carried_item = val; });

	vm->set_function("isItemCollidable", [&](ITEM_INFO* item)			 { return !!item->collidable; });
	vm->set_function("setItemCollidable", [&](ITEM_INFO* item, bool val) { item->collidable = val; });

	vm->set_function("getItemPosition", [&](ITEM_INFO* item) -> std::tuple<int, int, int>
	{
		return { item->pos.x_pos, item->pos.y_pos, item->pos.z_pos };
	});

	vm->set_function("setItemPosition", [&](ITEM_INFO* item, int x, int y, int z)
	{
		item->pos.x_pos = x;
		item->pos.y_pos = y;
		item->pos.z_pos = z;
	});

	vm->set_function("getItemRotation", [&](ITEM_INFO* item) -> std::tuple<int16_t, int16_t, int16_t>
	{
		return { item->pos.x_rot, item->pos.y_rot, item->pos.z_rot };
	});

	vm->set_function("setItemRotation", [&](ITEM_INFO* item, int16_t x, int16_t y, int16_t z)
	{
		item->pos.x_rot = x;
		item->pos.y_rot = y;
		item->pos.z_rot = z;
	});

	vm->set_function("getItemPR", [&](ITEM_INFO* item) -> std::tuple<int, int, int, int16_t, int16_t, int16_t>
	{
		return { item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, item->pos.x_rot, item->pos.y_rot, item->pos.z_rot };
	});

	vm->set_function("getItemBounds", [&](ITEM_INFO* item) -> std::tuple<int16_t, int16_t, int16_t, int16_t, int16_t, int16_t>
	{
		auto bounds = GetBoundsAccurate(item);

		return { bounds[0], bounds[2], bounds[4], bounds[1], bounds[3], bounds[5] };
	});
	
	vm->set_function("getItemBonePosition", [&](ITEM_INFO* item, int bone) -> std::tuple<int, int, int>
	{
		if (!item)
			return {};

		PHD_VECTOR vec {};

		get_lara_bone_pos(item, &vec, bone);

		return { vec.x, vec.y, vec.z };
	});

	vm->set_function("killCreature", [&](ITEM_INFO* item, bool explode, bool sync_explosion)
	{
		if (g_level->get_player_by_item(item) || (lara_item && game_level::LOCALPLAYER()->get_item() == lara_item))
			CreatureDie(item->id, explode, true, sync_explosion);
		else CreatureDie(item->id, explode, false, sync_explosion);
	});

	vm->set_function("getItemGravityStatus", [&](ITEM_INFO* item) { return bool(item->gravity_status); });
	vm->set_function("setItemGravityStatus", [&](ITEM_INFO* item, bool v) { item->gravity_status = v; });

	vm->set_function("getItemGoalAnim", [&](ITEM_INFO* item) { return item->goal_anim_state; });
	vm->set_function("setItemGoalAnim", [&](ITEM_INFO* item, int16_t v) { item->goal_anim_state = v; });

	vm->set_function("getItemCurrentAnim", [&](ITEM_INFO* item) { return item->current_anim_state; });
	vm->set_function("setItemCurrentAnim", [&](ITEM_INFO* item, int16_t v) { item->current_anim_state = v; });

	vm->set_function("getItemAnimID", [&](ITEM_INFO* item) { return item->anim_number; });
	vm->set_function("setItemAnimID", [&](ITEM_INFO* item, int16_t v) { item->anim_number = v; });

	vm->set_function("getItemAnimFrame", [&](ITEM_INFO* item) { return item->frame_number; });
	vm->set_function("setItemAnimFrame", [&](ITEM_INFO* item, int16_t v) { item->frame_number = v; });
}
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
	vm->set_function("getItemObjectID", [&](ITEM_INFO* item) { return item ? item->object_number : -1; });

	vm->set_function("getItemMeshBits", [&](ITEM_INFO* item)				{ return IsValidItem(item) ? item->mesh_bits : -1; });
	vm->set_function("setItemMeshBits", [&](ITEM_INFO* item, uint32_t val)	{ IsValidItem(item) ? (item->mesh_bits = val) : (0); });
	
	vm->set_function("getItemObjectID", [&](ITEM_INFO* item)				{ return IsValidItem(item) ? item->object_number : -1; });
	vm->set_function("setItemObjectID", [&](ITEM_INFO* item, int16_t val)   { IsValidItem(item) ? (item->object_number = val) : (0); });

	vm->set_function("getItemRoom", [&](ITEM_INFO* item)				{ return IsValidItem(item) ? item->room_number : 0; });
	vm->set_function("setItemRoom", [&](ITEM_INFO* item, int16_t val)	{ IsValidItem(item) ? (item->room_number = val) : (0); });
	
	vm->set_function("getItemSpeed", [&](ITEM_INFO* item)				{ return IsValidItem(item) ? item->speed : 0; });
	vm->set_function("setItemSpeed", [&](ITEM_INFO* item, int16_t val)  { IsValidItem(item) ? (item->speed = val) : (0); });

	vm->set_function("getItemFallspeed", [&](ITEM_INFO* item)				{ return IsValidItem(item) ? item->fallspeed : 0; });
	vm->set_function("setItemFallspeed", [&](ITEM_INFO* item, int16_t val)  { IsValidItem(item) ? (item->fallspeed = val) : (0); });

	vm->set_function("getItemHealth", [&](ITEM_INFO* item)				{ return IsValidItem(item) ? item->hit_points : DONT_TARGET; });
	vm->set_function("setItemHealth", [&](ITEM_INFO* item, int16_t val) { IsValidItem(item) ? (item->hit_points = val) : (0); });

	vm->set_function("getItemCarriedItem", [&](ITEM_INFO* item)				 { return IsValidItem(item) ? item->carried_item : NO_ITEM; });
	vm->set_function("setItemCarriedItem", [&](ITEM_INFO* item, int16_t val) { IsValidItem(item) ? (item->carried_item = val) : (0); });

	vm->set_function("isItemCollidable", [&](ITEM_INFO* item)			 { return IsValidItem(item) ? !!item->collidable : false; });
	vm->set_function("setItemCollidable", [&](ITEM_INFO* item, bool val) { IsValidItem(item) ? (item->collidable = uint16_t(val)) : (0); });

	vm->set_function("isItemIntelligent", [&](ITEM_INFO* item)				{ return IsValidItem(item) ? !!objects[item->object_number].intelligent : false; });
	vm->set_function("setItemIntelligent", [&](ITEM_INFO* item, bool val)	{ IsValidItem(item) ? (objects[item->object_number].intelligent = val) : (0); });

	vm->set_function("getItemPosition", [&](ITEM_INFO* item) -> std::tuple<int, int, int>
	{
		if (!IsValidItem(item))
			return { 0, 0, 0 };

		return { item->pos.x_pos, item->pos.y_pos, item->pos.z_pos };
	});

	vm->set_function("setItemPosition", [&](ITEM_INFO* item, int x, int y, int z)
	{
		if (!IsValidItem(item))
			return;

		item->pos.x_pos = x;
		item->pos.y_pos = y;
		item->pos.z_pos = z;
	});

	vm->set_function("getItemRotation", [&](ITEM_INFO* item) -> std::tuple<int16_t, int16_t, int16_t>
	{
		if (!IsValidItem(item))
			return { 0, 0, 0 };

		return { item->pos.x_rot, item->pos.y_rot, item->pos.z_rot };
	});

	vm->set_function("setItemRotation", [&](ITEM_INFO* item, int16_t x, int16_t y, int16_t z)
	{
		if (!IsValidItem(item))
			return;

		item->pos.x_rot = x;
		item->pos.y_rot = y;
		item->pos.z_rot = z;
	});

	vm->set_function("getItemPR", [&](ITEM_INFO* item) -> std::tuple<int, int, int, int16_t, int16_t, int16_t>
	{
		if (!IsValidItem(item))
			return {};

		return { item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, item->pos.x_rot, item->pos.y_rot, item->pos.z_rot };
	});

	vm->set_function("getItemBounds", [&](ITEM_INFO* item) -> std::tuple<int16_t, int16_t, int16_t, int16_t, int16_t, int16_t>
	{
		if (!IsValidItem(item))
			return {};

		auto bounds = GetBoundsAccurate(item);

		return { bounds[0], bounds[2], bounds[4], bounds[1], bounds[3], bounds[5] };
	});
	
	vm->set_function("getItemBonePosition", [&](ITEM_INFO* item, int bone) -> std::tuple<int, int, int>
	{
		if (!IsValidItem(item))
			return {};

		PHD_VECTOR vec {};

		get_lara_bone_pos(item, &vec, bone);

		return { vec.x, vec.y, vec.z };
	});

	vm->set_function("killCreature", [&](ITEM_INFO* item, bool explode, bool sync_explosion)
	{
		if (!IsValidItem(item))
			return;

		if (g_level->get_player_by_item(item) || (lara_item && game_level::LOCALPLAYER()->get_item() == lara_item))
			CreatureDie(item->id, explode, true, sync_explosion);
		else CreatureDie(item->id, explode, false, sync_explosion);
	});

	vm->set_function("getItemGravityStatus", [&](ITEM_INFO* item)			{ return IsValidItem(item) ? bool(item->gravity_status) : false; });
	vm->set_function("setItemGravityStatus", [&](ITEM_INFO* item, bool v)	{ IsValidItem(item) ? (item->gravity_status = v) : (0); });

	vm->set_function("getItemGoalAnim", [&](ITEM_INFO* item)			{ return IsValidItem(item) ? item->goal_anim_state : -1; });
	vm->set_function("setItemGoalAnim", [&](ITEM_INFO* item, int16_t v) { IsValidItem(item) ? (item->goal_anim_state = v) : (0); });

	vm->set_function("getItemCurrentAnim", [&](ITEM_INFO* item)				{ return IsValidItem(item) ? item->current_anim_state : -1; });
	vm->set_function("setItemCurrentAnim", [&](ITEM_INFO* item, int16_t v)	{ IsValidItem(item) ? (item->current_anim_state = v) : (0); });

	vm->set_function("getItemAnimID", [&](ITEM_INFO* item)				{ return IsValidItem(item) ? item->anim_number : -1; });
	vm->set_function("setItemAnimID", [&](ITEM_INFO* item, int16_t v)	{ IsValidItem(item) ? (item->anim_number = v) : (0); });

	vm->set_function("getItemAnimFrame", [&](ITEM_INFO* item)				{ return IsValidItem(item) ? item->frame_number : -1; });
	vm->set_function("setItemAnimFrame", [&](ITEM_INFO* item, int16_t v)	{ IsValidItem(item) ? (item->frame_number = v) : (0); });

	vm->set_function("getItemAI", [&](ITEM_INFO* item) { return IsValidItem(item) ? (item->active ? (CREATURE_INFO*)item->data : nullptr) : nullptr; });

	vm->set_function("getAIEnemy", [&](CREATURE_INFO* ai)					{ return ai ? ai->enemy : nullptr; });
	vm->set_function("setAIEnemy", [&](CREATURE_INFO* ai, ITEM_INFO* enemy)
	{
		if (!ai)
			return;

		ai->enemy = enemy;
		ai->hurt_by_lara = 1;	// fix me
	});
}
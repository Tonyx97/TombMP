import prof;
import utils;

#include <shared/defs.h>

#include <scripting/globals.h>
#include <specific/standard.h>

#include <game/moveblok.h>
#include <game/lot.h>

#include <mp/client.h>

#include "entity.h"

game_entity::game_entity(int stype, int16_t id, SYNC_ID sid)
{
	type = ENTITY_TYPE_LEVEL;
	subtype = stype;
	sync_id = sid;
	item = &items[item_id = id];
}

game_entity::game_entity(int16_t obj_id, int stype, SYNC_ID sid, const game_vec3d& vec)
{
	type = ENTITY_TYPE_LEVEL;
	subtype = stype;
	sync_id = sid;

	if (item_id = CreateItem(); item_id != NO_ITEM)
	{
		item = &items[item_id];

		item->room_number = NO_ROOM;

		set_obj_id(obj_id);
		set_new_room(vec.room);
		set_position(vec.pos);
		set_rotation(vec.rot);

		update_linked_lists();

		BasicSetupItem(item_id);

		const bool intelligent = objects[obj_id].intelligent || obj_id == SAVEGAME_CRYSTAL_ITEM;

		if (intelligent)
			set_new_active(true);

		update_linked_lists();

		if (intelligent)
		{
			item->touch_bits = 0;
			item->status = ACTIVE;

			EnableBaddieAI(item_id, 1);
		}

		spawn();
	}
}

game_entity::~game_entity()
{
}

void game_entity::sync(bool ignore_streamer)
{
	if (block_default_sync)
		return;

	switch (subtype)
	{
	case ENTITY_LEVEL_TYPE_BLOCK:
	{
		gns::sync::block info;

		utils::mem::move_ex(info.pos, item->pos.x_pos);
		utils::mem::move_ex(info.rot, item->pos.x_rot);

		info.sid = sync_id;
		info.ignore_streamer = ignore_streamer;
		info.room = get_room();
		info.anim = get_anim_id();
		info.frame = get_anim_frame();
		info.flags = get_flags();
		info.flags0 = get_item_flags(0);
		info.active = get_active();
		info.status = get_status();
		info.gravity_status = get_gravity_status();

		g_client->send_packet(ID_SYNC_BLOCK, info);

		break;
	}
	case ENTITY_LEVEL_TYPE_VEHICLE:
	{
		gns::sync::ai info;

		utils::mem::move_ex(info.pos, item->pos.x_pos);
		utils::mem::move_ex(info.rot, item->pos.x_rot);
		utils::mem::move_ex(info.local_pos, item->local_pos.x_pos);
		utils::mem::move_ex(info.local_rot, item->local_pos.x_rot);

		info.sid = sync_id;
		info.ignore_streamer = ignore_streamer;
		info.mesh_bits = get_mesh_bits();
		info.touch_bits = get_touch_bits();
		info.current_anim_state = get_current_anim_state();
		info.goal_anim_state = get_goal_anim_state();
		info.room = get_room();
		info.hp = get_health();
		info.anim = get_anim_id();
		info.frame = get_anim_frame();
		info.flags = get_flags();
		info.flags0 = get_item_flags(0);
		info.active = get_active();
		info.status = get_status();

		g_client->send_packet(ID_SYNC_VEHICLE, info);

		break;
	}
	case ENTITY_LEVEL_TYPE_AI:
	{
		gns::sync::ai info;

		utils::mem::move_ex(info.pos, item->pos.x_pos);
		utils::mem::move_ex(info.rot, item->pos.x_rot);

		info.sid = sync_id;
		info.ignore_streamer = ignore_streamer;
		info.mesh_bits = get_mesh_bits();
		info.touch_bits = get_touch_bits();
		info.current_anim_state = get_current_anim_state();
		info.goal_anim_state = get_goal_anim_state();
		info.room = get_room();
		info.hp = get_health();
		info.anim = get_anim_id();
		info.frame = get_anim_frame();
		info.flags = get_flags();
		info.flags0 = get_item_flags(0);
		info.active = get_active();
		info.status = get_status();

		g_client->send_packet(ID_SYNC_AI, info);

		break;
	}
	case ENTITY_LEVEL_TYPE_INTERACTIVE:
	{
		gns::sync::interactive info;

		info.sid = sync_id;
		info.ignore_streamer = ignore_streamer;
		info.obj_id = get_obj_id();
		info.anim = get_anim_id();
		info.frame = get_anim_frame();
		info.current_anim_state = get_current_anim_state();
		info.goal_anim_state = get_goal_anim_state();
		info.flags = get_flags();
		info.flags0 = get_item_flags(0);
		info.timer = get_timer();
		info.active = get_active();
		info.status = get_status();

		g_client->send_packet(ID_SYNC_INTERACTIVE, info);

		break;
	}
	case ENTITY_LEVEL_TYPE_TRAP:
	case ENTITY_LEVEL_TYPE_DOOR:
	case ENTITY_LEVEL_TYPE_ANIMATING:
	case ENTITY_LEVEL_TYPE_SPECIAL_FX:
	{
		gns::sync::base_info info;

		info.sid = sync_id;
		info.ignore_streamer = ignore_streamer;
		info.anim = get_anim_id();
		info.frame = get_anim_frame();
		info.current_anim_state = get_current_anim_state();
		info.goal_anim_state = get_goal_anim_state();
		info.flags = get_flags();
		info.flags0 = get_item_flags(0);
		info.timer = get_timer();
		info.active = get_active();
		info.status = get_status();

		g_client->send_packet(ID_SYNC_OTHERS, info);

		break;
	}
	}
}

void game_entity::force_sync(bool ignore_streamer)
{
	const bool old_sync = block_default_sync;

	block_default_sync = false;

	sync(ignore_streamer);

	block_default_sync = old_sync;
}

void game_entity::update_by_subtype(const subtype_update_info& info)
{
	switch (subtype)
	{
	case ENTITY_LEVEL_TYPE_BLOCK:
	{
		auto old_pos = get_position();
		auto old_room = get_room();

		const bool move_floor_change = (old_pos.x != info.pos.x ||
										old_pos.y != info.pos.y ||
										old_pos.z != info.pos.z);

		const bool is_moving = (!get_active() && info.new_active);

		if (move_floor_change && !is_moving)
		{
			AlterFloorHeight(old_pos.x, old_pos.y, old_pos.z, old_room, 1024);
			AdjustStopperFlag(old_pos.x, old_pos.y, old_pos.z, old_room, info.flags0, 1);

			AlterFloorHeight(info.pos.x, info.pos.y, info.pos.z, info.room, -1024);
			AdjustStopperFlag(info.pos.x, info.pos.y, info.pos.z, info.room, info.flags0 + 0x8000, 0);
		}
		else if (is_moving)
		{
			if (move_floor_change)
			{
				AlterFloorHeight(old_pos.x, old_pos.y, old_pos.z, old_room, 1024);
				AdjustStopperFlag(old_pos.x, old_pos.y, old_pos.z, old_room, info.flags0, 1);
			}

			if (move_floor_change)
			{
				AlterFloorHeight(info.pos.x, info.pos.y, info.pos.z, info.room, -1024);
				AdjustStopperFlag(info.pos.x, info.pos.y, info.pos.z, info.room, info.flags0 + 0x8000, 0);
			}
		}

		break;
	}
	case ENTITY_LEVEL_TYPE_AI:
	{
		if (!get_active() && info.new_active)
		{
			if (auto curr_status = get_status(); curr_status == NOT_ACTIVE)
			{
				if (get_obj_id() != TARGETS)
					EnableBaddieAI(item_id, 1);
			}
			else if (curr_status == INVISIBLE)
				EnableBaddieAI(item_id, 0);
		}

		if (!is_dead() && info.hp <= 0)
			CreatureDie(item_id, info.flags & KILLED_ITEM, false, false);

		break;
	}
	case ENTITY_LEVEL_TYPE_INTERACTIVE:
	case ENTITY_LEVEL_TYPE_TRAP:
	case ENTITY_LEVEL_TYPE_DOOR:
	case ENTITY_LEVEL_TYPE_ANIMATING:
	case ENTITY_LEVEL_TYPE_SPECIAL_FX:
	{
		break;
	}
	}
}

bool game_entity::spawn()
{
	return (spawned = true);
}
import prof;

#include <shared/defs.h>
#include <shared/scripting/resource.h>
#include <shared/scripting/resource_system.h>

#include <scripting/globals.h>
#include <specific/standard.h>

#include <mp/client.h>

#include "player.h"

game_player::game_player(PLAYER_ID id, SYNC_ID sid) : id(id)
{
	type = ENTITY_TYPE_PLAYER;
	sync_id = sid;

	set_local();
}

game_player::game_player(PLAYER_ID id, SYNC_ID sid, const game_vec3d& spawn_vec) : id(id)
{
	type = ENTITY_TYPE_PLAYER;
	sync_id = sid;

	if (item_id = CreateItem(); item_id != NO_ITEM)
	{
		item = &items[item_id];

		utils::mem::move(item->pos.x_pos, spawn_vec.pos);
		utils::mem::move(item->pos.x_rot, spawn_vec.rot);

		set_entity_flags(ENTITY_FLAG_INVISIBLE);

		item->object_number = LARA;
		item->room_number = old_room = new_room = spawn_vec.room;
		item->box_number = spawn_vec.box;
		item->anim_number = objects[item->object_number].anim_index;
		item->frame_number = anims[item->anim_number].frame_base;
		item->current_anim_state = item->goal_anim_state = anims[item->anim_number].current_anim_state;
		item->required_anim_state = 0;
		item->pos.x_rot = item->pos.y_rot = item->pos.z_rot = 0;
		item->speed = item->fallspeed = 0;
		item->active = 0;
		item->gravity_status = item->hit_status = item->looked_at = item->really_active = item->ai_bits = item->dynamic_light = 0;
		item->item_flags[0] = item->item_flags[1] = item->item_flags[2] = item->item_flags[3] = 0;
		item->hit_points = objects[item->object_number].hit_points;
		item->collidable = 0;
		item->clear_body = 0;
		item->timer = 0;
		item->mesh_bits = 0xffffffff;
		item->touch_bits = 0;
		item->after_death = 0;
		item->il.init = 0;
		item->fired_weapon = 0;
		item->data = nullptr;
		item->carried_item = NO_ITEM;

		auto r = &room[item->room_number];

		item->next_item = r->item_number;

		r->item_number = item_id;

		InitialiseHair((vec3d*)hair, (int_vec3*)hair_vel);
	}
}

game_player::~game_player()
{
}

bool game_player::spawn()
{
	item->active = ACTIVE;
	item->next_active = next_item_active;
	item->collidable = 1;

	next_item_active = get_item_id();

	remove_entity_flags(ENTITY_FLAG_INVISIBLE);

	prof::print(GREEN, "[PLAYERS] We spawned player {:#x}", get_id());

	return (spawned = true);
}

void game_player::update_localplayer_instance_info(bool clear)
{
	if (!local)
		return;

	if (clear)
	{
		item = nullptr;
		item_id = NO_ITEM;
	}
	else
	{
		item = lara_item;
		item_id = lara.item_number;
	}

	id = g_client->get_id();
	sync_id = g_client->get_sync_id();
	name = g_client->get_name();

	g_resource->update_global(scripting::globals::LOCALPLAYER, this);
	g_resource->update_global(scripting::globals::LOCALPLAYER_ITEM, item);
}
import prof;

#include <shared/defs.h>
#include <shared/scripting/resource_system.h>

#include <mp/client.h>
#include <mp/game/entity.h>
#include <mp/game/player.h>
#include <mp/game/level.h>

#include <game/pickup.h>
#include <game/control.h>
#include <game/moveblok.h>
#include <game/lot.h>
#include <game/types.h>

#include "hl_defs.h"

void sync_handlers::handle_packet(uint16_t pid)
{
	switch (pid)
	{
	case ID_SYNC_LEVEL_ENTITIES:		return on_level_entities();
	case ID_SYNC_REQUEST_STREAM_INFO:	return on_request_stream_info();
	case ID_SYNC_STREAM_INFO:			return on_stream_info();
	case ID_SYNC_INITIAL_INFO:			return on_initial_info();
	case ID_SYNC_BLOCK:					return on_block();
	case ID_SYNC_AI:					return on_ai();
	case ID_SYNC_VEHICLE:				return on_vehicle();
	case ID_SYNC_INTERACTIVE:			return on_interactive();
	case ID_SYNC_OTHERS:				return on_others();
	case ID_SYNC_ATTACHMENTS:			return on_attachments();
	case ID_SYNC_KILL:					return on_kill();
	}
}

void sync_handlers::on_level_entities()
{
	auto bs = g_client->get_current_bs();

	if (int size = 0; bs->Read(size) && size >= 1)
	{
		for (int i = 0; i < size; ++i)
		{
			gns::sync::level_entity_initial_basic_sync info; bs->Read(info);

			if (auto entity = g_level->add_level_entity(info.subtype, info.id, info.sid))
				entity->set_as_level_entity();
		}
	}

	// delete default level items that should be synchronized and
	// we didn't get the info from (because they are not in the server)
	
	for (int i = 0; i < level_items; ++i)
		if (auto item = &items[i]; g_level->get_level_entity_subtype_from_obj_id(item->object_number) != ENTITY_LEVEL_TYPE_NONE)
			if (!g_level->get_entity_by_item(item))
				KillItem(i, false);
}

void sync_handlers::on_request_stream_info()
{
	auto localplayer = game_level::LOCALPLAYER();

	if (!g_client->is_ready() || !localplayer->get_item())
		return;
	
	auto bs = g_client->create_packet(ID_SYNC_REQUEST_STREAM_INFO);

	bs->Write(g_level->get_instanced_level_entities_count());

	g_level->for_each_level_entity([&](game_entity* entity)
	{
		bs->Write(gns::sync::stream_sync_info { .sid = entity->get_sync_id(), .distance = entity->get_position().distance(localplayer->get_position()) });
	});

	g_client->send_packet(bs);
}

void sync_handlers::on_stream_info()
{
	auto bs = g_client->get_current_bs();

	int size = 0; bs->Read(size);

	for (int i = 0; i < size; ++i)
	{
		gns::sync::stream_info info; bs->Read(info);

		if (info.add)
			g_level->add_streamed_entity(info.sid);
		else g_level->remove_streamed_entity(info.sid);
	}
}

void sync_handlers::on_initial_info()
{
	auto bs = g_client->get_current_bs();

	int size = 0; bs->Read(size);

	for (int i = 0; i < size; ++i)
	{
		gns::sync::base_info info; bs->Read(info);

		if (auto entity = g_level->get_entity_by_sid(info.sid))
		{
			subtype_update_info sui
			{
				.pos = info.pos,
				.room = info.room,
				.hp = info.hp,
				.flags = info.flags,
				.flags0 = info.flags0,
				.new_active = bool(info.active)
			};

			entity->update_by_subtype(sui);

			switch (entity->get_subtype())
			{
			case ENTITY_LEVEL_TYPE_BLOCK:
			{
				entity->set_position(info.pos);
				entity->set_rotation(info.rot);
				entity->set_new_room(info.room);
				entity->set_anim_id(info.anim);
				entity->set_anim_frame(info.frame);
				entity->set_flags(info.flags);
				entity->set_item_flags(0, info.flags0);
				entity->set_new_active(info.active);
				entity->set_status(info.status);
				entity->set_gravity_status(info.gravity_status);

				break;
			}
			case ENTITY_LEVEL_TYPE_AI:
			{
				entity->set_position(info.pos);
				entity->set_rotation(info.rot);
				entity->set_new_room(info.room);
				entity->set_health(info.hp);
				entity->set_anim_id(info.anim);
				entity->set_anim_frame(info.frame);
				entity->set_current_anim_state(info.current_anim_state);
				entity->set_goal_anim_state(info.goal_anim_state);
				entity->set_flags(info.flags);
				entity->set_item_flags(0, info.flags0);
				entity->set_new_active(info.active);
				entity->set_status(info.status);

				break;
			}
			case ENTITY_LEVEL_TYPE_VEHICLE:
			{
				entity->set_position(info.pos);
				entity->set_rotation(info.rot);
				entity->set_local_position(info.local_pos);
				entity->set_local_rotation(info.local_rot);
				entity->set_new_room(info.room);
				entity->set_health(info.hp);
				entity->set_anim_id(info.anim);
				entity->set_anim_frame(info.frame);
				entity->set_current_anim_state(info.current_anim_state);
				entity->set_goal_anim_state(info.goal_anim_state);
				entity->set_flags(info.flags);
				entity->set_item_flags(0, info.flags0);
				entity->set_new_active(info.active);
				entity->set_status(info.status);

				break;
			}
			case ENTITY_LEVEL_TYPE_INTERACTIVE:
			{
				entity->set_obj_id(info.obj_id);
				entity->set_anim_id(info.anim);
				entity->set_anim_frame(info.frame);
				entity->set_current_anim_state(info.current_anim_state);
				entity->set_goal_anim_state(info.goal_anim_state);
				entity->set_timer(info.timer);
				entity->set_flags(info.flags);
				entity->set_item_flags(0, info.flags0);
				entity->set_new_active(info.active);
				entity->set_status(info.status);

				break;
			}
			case ENTITY_LEVEL_TYPE_TRAP:
			case ENTITY_LEVEL_TYPE_DOOR:
			case ENTITY_LEVEL_TYPE_ANIMATING:
			case ENTITY_LEVEL_TYPE_SPECIAL_FX:
			{
				entity->set_anim_id(info.anim);
				entity->set_anim_frame(info.frame);
				entity->set_current_anim_state(info.current_anim_state);
				entity->set_goal_anim_state(info.goal_anim_state);
				entity->set_timer(info.timer);
				entity->set_flags(info.flags);
				entity->set_item_flags(0, info.flags0);
				entity->set_new_active(info.active);
				entity->set_status(info.status);

				break;
			}
			}

			entity->spawn();
			entity->update_linked_lists();
		}
	}
}

void sync_handlers::on_block()
{
	gns::sync::block info; g_client->read_packet_ex(info);

	if (auto entity = g_level->get_entity_by_sid(info.sid))
	{
		subtype_update_info sui
		{
			.pos = info.pos,
			.room = info.room,
			.hp = info.hp,
			.flags = info.flags,
			.flags0 = info.flags0,
			.new_active = bool(info.active)
		};

		entity->update_by_subtype(sui);

		entity->set_position(info.pos);
		entity->set_rotation(info.rot);
		entity->set_new_room(info.room);
		entity->set_anim_id(info.anim);
		entity->set_anim_frame(info.frame);
		entity->set_flags(info.flags);
		entity->set_item_flags(0, info.flags0);
		entity->set_new_active(info.active);
		entity->set_status(info.status);
		entity->set_gravity_status(info.gravity_status);

		entity->update_linked_lists();
	}
}

void sync_handlers::on_ai()
{
	gns::sync::ai info; g_client->read_packet_ex(info);

	if (auto entity = g_level->get_entity_by_sid(info.sid))
	{
		subtype_update_info sui
		{
			.hp = info.hp,
			.flags = info.flags,
			.new_active = bool(info.active)
		};

		entity->update_by_subtype(sui);

		entity->set_position(info.pos);
		entity->set_rotation(info.rot);
		entity->set_mesh_bits(info.mesh_bits);
		entity->set_touch_bits(info.touch_bits);
		entity->set_new_room(info.room);
		entity->set_health(info.hp);
		entity->set_anim_id(info.anim);
		entity->set_anim_frame(info.frame);
		entity->set_current_anim_state(info.current_anim_state);
		entity->set_goal_anim_state(info.goal_anim_state);
		entity->set_flags(info.flags);
		entity->set_item_flags(0, info.flags0);
		entity->set_new_active(info.active);
		entity->set_status(info.status);
		
		entity->update_linked_lists();
	}
}

void sync_handlers::on_vehicle()
{
	gns::sync::vehicle info; g_client->read_packet_ex(info);

	if (auto entity = g_level->get_entity_by_sid(info.sid))
	{
		subtype_update_info sui
		{
			.hp = info.hp,
			.flags = info.flags,
			.new_active = bool(info.active)
		};

		entity->update_by_subtype(sui);

		//entity->set_next_position(info.pos);
		entity->set_position(info.pos);
		entity->set_rotation(info.rot);
		entity->set_local_position(info.local_pos);
		entity->set_local_rotation(info.local_rot);
		entity->set_mesh_bits(info.mesh_bits);
		entity->set_touch_bits(info.touch_bits);
		entity->set_new_room(info.room);
		entity->set_health(info.hp);
		entity->set_anim_id(info.anim);
		entity->set_anim_frame(info.frame);
		entity->set_current_anim_state(info.current_anim_state);
		entity->set_goal_anim_state(info.goal_anim_state);
		entity->set_flags(info.flags);
		entity->set_item_flags(0, info.flags0);
		entity->set_new_active(info.active);
		entity->set_status(info.status);

		entity->update_linked_lists();
	}
}

void sync_handlers::on_interactive()
{
	gns::sync::interactive info; g_client->read_packet_ex(info);

	if (auto entity = g_level->get_entity_by_sid(info.sid))
	{
		entity->set_obj_id(info.obj_id);
		entity->set_anim_id(info.anim);
		entity->set_anim_frame(info.frame);
		entity->set_current_anim_state(info.current_anim_state);
		entity->set_goal_anim_state(info.goal_anim_state);
		entity->set_flags(info.flags);
		entity->set_timer(info.timer);
		entity->set_item_flags(0, info.flags0);
		entity->set_new_active(info.active);
		entity->set_status(info.status);

		entity->update_linked_lists();
	}
}

void sync_handlers::on_others()
{
	gns::sync::base_info info; g_client->read_packet_ex(info);

	if (auto entity = g_level->get_entity_by_sid(info.sid))
	{
		entity->set_anim_id(info.anim);
		entity->set_anim_frame(info.frame);
		entity->set_current_anim_state(info.current_anim_state);
		entity->set_goal_anim_state(info.goal_anim_state);
		entity->set_flags(info.flags);
		entity->set_timer(info.timer);
		entity->set_item_flags(0, info.flags0);
		entity->set_new_active(info.active);
		entity->set_status(info.status);

		entity->update_linked_lists();
	}
}

void sync_handlers::on_attachments()
{
	auto bs = g_client->get_current_bs();

	int size = 0; bs->Read(size);

	for (int i = 0; i < size; ++i)
	{
		SYNC_ID a_sid,
				b_sid;
		
		bs->Read(a_sid);
		bs->Read(b_sid);

		int_vec3 local_pos; bs->Read(local_pos);
		short_vec3 local_rot; bs->Read(local_rot);

		if (auto a = g_level->get_entity_by_sid(a_sid))
			if (auto b = g_level->get_entity_by_sid(b_sid))
				attach_entities(a->get_item(), b->get_item(), local_pos, local_rot);
	}
}

void sync_handlers::on_kill()
{
	SYNC_ID sid; g_client->read_packet_ex(sid);

	if (auto entity = g_level->get_entity_by_sid(sid))
		g_level->remove_entity(entity);
}
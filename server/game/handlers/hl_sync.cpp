import prof;

#include <shared/defs.h>

#include <server/server.h>

#include <net/net_player.h>

#include <game/entity.h>
#include <game/player.h>
#include <game/level.h>

#include "hl_defs.h"

void sync_handlers::handle_packet(net_player* n_player, uint16_t pid)
{
	if (!n_player)
		return;

	switch (pid)
	{
	case ID_SYNC_LEVEL_ENTITIES:		return on_level_entities(n_player);
	case ID_SYNC_SPAWNED_ENTITIES:		return on_spawned_entities(n_player);
	case ID_SYNC_REQUEST_STREAM_INFO:	return on_request_stream_info(n_player);
	case ID_SYNC_OWNERSHIP:				return on_ownership(n_player);
	case ID_SYNC_BLOCK:					return on_block(n_player);
	case ID_SYNC_AI:					return on_ai(n_player);
	case ID_SYNC_VEHICLE:				return on_vehicle(n_player);
	case ID_SYNC_INTERACTIVE:			return on_interactive(n_player);
	case ID_SYNC_OTHERS:				return on_others(n_player);
	case ID_SYNC_KILL:					return on_kill(n_player);
	}
}

void sync_handlers::on_level_entities(net_player* n_player)
{
	if (g_level->are_level_entities_synced())
	{
		auto bs = g_server->create_packet(ID_SYNC_LEVEL_ENTITIES);

		int size = 0;

		g_level->for_each_level_entity([&](game_entity* entity)
		{
			if (entity->is_level_entity())
				++size;
		});

		bs->Write(size);

		g_level->for_each_level_entity([&](game_entity* entity)
		{
			if (entity->is_level_entity())
				bs->Write(gns::sync::level_entity_initial_basic_sync
				{
					.sid = entity->get_sync_id(),
					.subtype = entity->get_subtype(),
					.id = entity->get_item_id()
				});
		});

		g_server->send_packet_broadcast(bs, n_player->get_sys_address(), false);

		g_level->send_initial_level_entities_info(n_player->get_player());

		return;
	}

	auto in_bs = g_server->get_current_bs();

	if (int size = 0; in_bs->Read(size) && size >= 0)
	{
		for (int i = 0; i < size; ++i)
			if (game_vec3d spawn_loc; in_bs->Read(spawn_loc))
				g_level->add_player_spawn(spawn_loc);
	}
	else return;

	if (int size = 0; in_bs->Read(size) && size >= 1)
	{
		for (int i = 0; i < size; ++i)
			if (bool loaded; in_bs->Read(loaded) && loaded)
				g_level->set_object_loaded(int16_t(i));
	}
	else return;

	if (int size = 0; in_bs->Read(size) && size >= 1)
	{
		auto out_bs = g_server->create_packet(ID_SYNC_LEVEL_ENTITIES);

		out_bs->Write(size);

		for (int i = 0; i < size; ++i)
		{
			gns::sync::level_entity_initial_basic_sync info; in_bs->Read(info);

			if (auto entity = g_level->add_level_entity(info.subtype, info.id))
			{
				entity->set_as_level_entity();

				info.sid = entity->get_sync_id();

				out_bs->Write(info);
			}
		}

		g_level->set_level_entities_synced();

		g_server->send_packet_broadcast(out_bs);

		g_level->send_initial_level_entities_info(n_player->get_player());
	}
}

void sync_handlers::on_spawned_entities(net_player* n_player)
{
	g_level->for_each_level_entity([&](game_entity* entity)
	{
		if (!entity->is_level_entity())
			g_level->spawn_entity_for_player(n_player, entity);
	});

	g_level->sync_attachments_for_player(n_player->get_player());
}

void sync_handlers::on_request_stream_info(net_player* n_player)
{
	auto bs = g_server->get_current_bs();
	auto player = n_player->get_player();

	int size = 0; bs->Read(size);

	for (int i = 0; i < size; ++i)
	{
		gns::sync::stream_sync_info info; bs->Read(info);

		if (player->is_spawned())
			player->add_stream_info(info);
	}

	player->set_streaming_synced();
}

void sync_handlers::on_ownership(net_player* n_player)
{
	gns::sync::ownership info; g_server->read_packet_ex(info);

	if (auto entity = g_level->get_entity_by_sid(info.sid))
	{
		auto new_player = n_player->get_player(),
			 old_player = entity->get_streamer();

		if (entity->is_streaming_locked())
			if (new_player != old_player)
				return;

		if (info.acquire)
		{
			if (old_player)
				old_player->transfer_entity_ownership(entity, new_player, info.timeout);
			else new_player->acquire_entity_ownership(entity, info.timeout);
		}
		else new_player->transfer_entity_ownership(entity, nullptr, info.timeout);
	}
}

void sync_handlers::on_block(net_player* n_player)
{
	gns::sync::block info; g_server->read_packet_ex(info);

	auto entity = g_level->get_entity_by_sid(info.sid);

	if (entity && (info.ignore_streamer || g_level->is_entity_streamed_by(entity, n_player->get_player())))
	{
		entity->set_position(info.pos);
		entity->set_rotation(info.rot);
		entity->set_room(info.room);
		entity->set_anim_id(info.anim);
		entity->set_anim_frame(info.frame);
		entity->set_flags(info.flags);
		entity->set_item_flags(0, info.flags0);
		entity->set_active(info.active);
		entity->set_status(info.status);
		entity->set_gravity_status(info.gravity_status);
		entity->spawn();

		g_server->send_packet_broadcast_ex(ID_SYNC_BLOCK, n_player->get_sys_address(), true, info);
	}
}

void sync_handlers::on_ai(net_player* n_player)
{
	gns::sync::ai info; g_server->read_packet_ex(info);

	auto entity = g_level->get_entity_by_sid(info.sid);

	if (entity && (info.ignore_streamer || g_level->is_entity_streamed_by(entity, n_player->get_player())))
	{
		entity->set_position(info.pos);
		entity->set_rotation(info.rot);
		entity->set_mesh_bits(info.mesh_bits);
		entity->set_touch_bits(info.touch_bits);
		entity->set_room(info.room);
		entity->set_health(info.hp);
		entity->set_anim_id(info.anim);
		entity->set_anim_frame(info.frame);
		entity->set_current_anim_state(info.current_anim_state);
		entity->set_goal_anim_state(info.goal_anim_state);
		entity->set_flags(info.flags);
		entity->set_item_flags(0, info.flags0);
		entity->set_active(info.active);
		entity->set_status(info.status);
		entity->spawn();

		g_server->send_packet_broadcast_ex(ID_SYNC_AI, n_player->get_sys_address(), true, info);
	}
}

void sync_handlers::on_vehicle(net_player* n_player)
{
	gns::sync::vehicle info; g_server->read_packet_ex(info);

	auto entity = g_level->get_entity_by_sid(info.sid);

	if (entity && (info.ignore_streamer || g_level->is_entity_streamed_by(entity, n_player->get_player())))
	{
		entity->set_position(info.pos);
		entity->set_rotation(info.rot);
		entity->set_mesh_bits(info.mesh_bits);
		entity->set_touch_bits(info.touch_bits);
		entity->set_room(info.room);
		entity->set_health(info.hp);
		entity->set_anim_id(info.anim);
		entity->set_anim_frame(info.frame);
		entity->set_current_anim_state(info.current_anim_state);
		entity->set_goal_anim_state(info.goal_anim_state);
		entity->set_flags(info.flags);
		entity->set_item_flags(0, info.flags0);
		entity->set_active(info.active);
		entity->set_status(info.status);
		entity->spawn();

		g_server->send_packet_broadcast_ex(ID_SYNC_VEHICLE, n_player->get_sys_address(), true, info);
	}
}

void sync_handlers::on_interactive(net_player* n_player)
{
	gns::sync::interactive info; g_server->read_packet_ex(info);

	auto entity = g_level->get_entity_by_sid(info.sid);

	if (entity && (info.ignore_streamer || g_level->is_entity_streamed_by(entity, n_player->get_player())))
	{
		entity->set_obj_id(info.obj_id);
		entity->set_anim_id(info.anim);
		entity->set_anim_frame(info.frame);
		entity->set_current_anim_state(info.current_anim_state);
		entity->set_goal_anim_state(info.goal_anim_state);
		entity->set_timer(info.timer);
		entity->set_flags(info.flags);
		entity->set_item_flags(0, info.flags0);
		entity->set_active(info.active);
		entity->set_status(info.status);
		entity->spawn();

		g_server->send_packet_broadcast_ex(ID_SYNC_INTERACTIVE, n_player->get_sys_address(), true, info);
	}
}

void sync_handlers::on_others(net_player* n_player)
{
	gns::sync::base_info info; g_server->read_packet_ex(info);

	auto entity = g_level->get_entity_by_sid(info.sid);

	if (entity && (info.ignore_streamer || g_level->is_entity_streamed_by(entity, n_player->get_player())))
	{
		entity->set_anim_id(info.anim);
		entity->set_anim_frame(info.frame);
		entity->set_current_anim_state(info.current_anim_state);
		entity->set_goal_anim_state(info.goal_anim_state);
		entity->set_timer(info.timer);
		entity->set_flags(info.flags);
		entity->set_item_flags(0, info.flags0);
		entity->set_active(info.active);
		entity->set_status(info.status);
		entity->spawn();

		g_server->send_packet_broadcast_ex(ID_SYNC_OTHERS, n_player->get_sys_address(), true, info);
	}
}

void sync_handlers::on_kill(net_player* n_player)
{
	SYNC_ID sid; g_server->read_packet_ex(sid);

	if (auto entity = g_level->get_entity_by_sid(sid))
	{
		g_server->send_packet_broadcast_ex(ID_SYNC_KILL, n_player->get_sys_address(), true, sid);

		g_level->remove_entity(entity);
	}
}
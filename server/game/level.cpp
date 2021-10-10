import utils;
import prof;

#include <shared/defs.h>

#include <net/net_player.h>

#include <server/server.h>

#include "entity.h"
#include "player.h"
#include "level.h"

game_level::game_level()
{
	mt.seed(__rdtsc());
}

game_level::~game_level()
{
	for (const auto& [sid, entity_base] : all_entities)
		delete entity_base;
}

game_entity_base* game_level::add_entity_base(game_entity_base* base)
{
	bool created = false;

	switch (base->get_type())
	{
	case ENTITY_TYPE_PLAYER:
	{
		if (auto player = utils::rtti::safe_cast<game_player>(base))
		{
			players.insert({ player->get_id(), player });
			instances.players.insert(player);

			created = true;
		}

		break;
	}
	case ENTITY_TYPE_LEVEL:
	{
		if (auto entity = utils::rtti::safe_cast<game_entity>(base))
		{
			instances.level_entities.insert(entity);
			created = true;
		}

		break;
	}
	}

	if (!created)
	{
		delete base;
		return nullptr;
	}

	all_entities.insert({ base->get_sync_id(), base });

	return base;
}

game_player* game_level::add_player(net_player* n_player)
{
	return utils::rtti::safe_cast<game_player>(add_entity_base(new game_player(n_player, generate_sync_id())));
}

game_entity* game_level::add_level_entity(int subtype, int16_t item_id)
{
	return utils::rtti::safe_cast<game_entity>(add_entity_base(new game_entity(subtype, item_id, generate_sync_id())));
}

game_entity* game_level::spawn_level_entity(int16_t obj_id, const game_vec3d& vec)
{
	if (!is_object_loaded(obj_id))
		return nullptr;

	if (auto subtype = g_level->get_level_entity_subtype_from_obj_id(obj_id); subtype != ENTITY_LEVEL_TYPE_NONE)
	{
		auto entity = add_level_entity(subtype);

		entity->set_obj_id(obj_id);
		entity->set_position(vec.pos);
		entity->set_rotation(vec.rot);
		entity->set_room(vec.room);
		entity->set_shade(int16_t(0x4210 | 0x8000));
		entity->spawn();

		g_server->send_packet_broadcast(ID_ENTITY_SPAWN, gns::entity::spawn
		{
			.vec = vec,
			.sid = entity->get_sync_id(),
			.obj_id = entity->get_obj_id()
		});
		
		return entity;
	}

	return nullptr;
}

void game_level::remove_entity(game_entity_base* base)
{
	switch (base->get_type())
	{
	case ENTITY_TYPE_PLAYER:
	{
		if (auto player = utils::rtti::safe_cast<game_player>(base))
		{
			instances.players.erase(player);
			players.erase(player->get_id());
		}

		break;
	}
	case ENTITY_TYPE_LEVEL:
	{
		if (auto entity = utils::rtti::safe_cast<game_entity>(base))
			instances.level_entities.erase(entity);

		break;
	}
	default: return;
	}

	all_entities.erase(base->get_sync_id());

	delete base;
}

void game_level::remove_all_level_entities()
{
	for (auto entity : instances.level_entities)
	{
		all_entities.erase(entity->get_sync_id());

		delete entity;
	}

	instances.level_entities.clear();
}

void game_level::reset_level_data()
{
	for (auto player : instances.players)
	{
		player->despawn();
		player->clear_streamed_entities();
	}

	remove_all_level_entities();

	attachments.clear();
	loaded_objects.clear();
	player_spawns.clear();

	level_entities_synced = false;
}

void game_level::update_entity_streamers()
{
	if (instances.players.empty() || instances.level_entities.empty())
		return;

	// let's make sure all players sent the response

	for (auto player : instances.players)
	{
		if (!player->is_spawned())
			continue;

		if (!player->is_streaming_synced())
			return;
	}

	// check which player is entity to which entity and assign proper streamer to the entities

	prof::print(WHITE, "-----------------------------------------------------");

	for (auto entity : instances.level_entities)
	{
		if (entity->is_streaming_locked())
			continue;

		if (entity->get_streaming_timeout() > 0)
		{
			entity->consume_streaming_timeout();

			if (entity->get_streaming_timeout() <= 0)
			{
				if (auto old_player = entity->get_streamer())
					old_player->transfer_entity_ownership(entity, nullptr);
			}

			continue;
		}

		bool use_distance_method = true;

		switch (entity->get_subtype())
		{
		case ENTITY_LEVEL_TYPE_BLOCK:
		case ENTITY_LEVEL_TYPE_INTERACTIVE:
		case ENTITY_LEVEL_TYPE_TRAP:
		case ENTITY_LEVEL_TYPE_DOOR:
		case ENTITY_LEVEL_TYPE_ANIMATING:
		case ENTITY_LEVEL_TYPE_VEHICLE:
			//case ENTITY_LEVEL_TYPE_AI:
		case ENTITY_LEVEL_TYPE_SPECIAL_FX: use_distance_method = false; break;
		}

		if (!use_distance_method)
			continue;

		auto entity_sid = entity->get_sync_id();
		auto old_player = entity->get_streamer();

		game_player* new_player = nullptr;

		int best_distance = INT32_MAX;

		for (auto player : instances.players)
		{
			if (auto info = player->get_stream_info_for_entity(entity_sid))
			{
				if (info->distance < best_distance)
				{
					best_distance = info->distance;
					new_player = player;
				}
			}
		}

		if (new_player && best_distance < 1024 * 20)	// testing
		{
			if (new_player != old_player)
			{
				if (old_player)
					old_player->remove_streamed_entity(entity);

				new_player->add_streamed_entity(entity);

				entity->set_streamer(new_player);
			}
		}
		else if (old_player)
		{
			old_player->remove_streamed_entity(entity);
			entity->reset_streaming_info();
		}
	}

	// send the list of the new entities that each player streams

	for (auto player : instances.players)
		player->send_stream_info();
}

void game_level::send_initial_level_entities_info(game_player* player)
{
	auto n_player = player->get_net();

	{
		auto bs = g_server->create_packet(ID_SYNC_INITIAL_INFO);

		bs->Write(static_cast<int>(instances.level_entities.size()));

		for (auto entity : instances.level_entities)
		{
			if (!entity->is_spawned())
				continue;

			gns::sync::base_info info;

			info.ignore_streamer = true;
			info.sid = entity->get_sync_id();
			info.pos = entity->get_position();
			info.rot = entity->get_rotation();
			info.local_pos = entity->get_local_position();
			info.local_rot = entity->get_local_rotation();
			info.mesh_bits = entity->get_mesh_bits();
			info.touch_bits = entity->get_touch_bits();
			info.obj_id = entity->get_obj_id();
			info.room = entity->get_room();
			info.hp = entity->get_health();
			info.anim = entity->get_anim_id();
			info.frame = entity->get_anim_frame();
			info.current_anim_state = entity->get_current_anim_state();
			info.goal_anim_state = entity->get_goal_anim_state();
			info.timer = entity->get_timer();
			info.flags = entity->get_flags();
			info.flags0 = entity->get_item_flags(0);
			info.shade = entity->get_shade();
			info.active = entity->is_active();
			info.status = entity->get_status();
			info.gravity_status = entity->get_gravity_status();

			bs->Write(info);
		}

		g_server->send_packet_broadcast(bs, n_player->get_sys_address(), false);
	}
}

void game_level::sync_attachments_for_player(game_player* player)
{
	auto n_player = player->get_net();
	auto bs = g_server->create_packet(ID_SYNC_ATTACHMENTS);

	bs->Write(static_cast<int>(attachments.size()));

	for (const auto& [a, b, local_pos, local_rot] : attachments)
	{
		bs->Write(a->get_sync_id());
		bs->Write(b->get_sync_id());
		bs->Write(local_pos);
		bs->Write(local_rot);
	}

	g_server->send_packet_broadcast(bs, n_player->get_sys_address(), false);
}

void game_level::add_attachment(game_entity* a, game_entity* b, const int_vec3& local_pos, const short_vec3& local_rot)
{
	attachments.push_back({ a, b, local_pos, local_rot });
}

bool game_level::spawn_entity_for_player(net_player* n_player, game_entity* entity)
{
	if (!entity || !n_player)
		return false;

	return g_server->send_packet_broadcast_ex(ID_ENTITY_SPAWN, n_player->get_sys_address(), false, gns::entity::spawn
	{
		.vec = 
		{
			.pos = entity->get_position(),
			.rot = entity->get_rotation(),
			.room = entity->get_room()
		},
		.sid = entity->get_sync_id(),
		.obj_id = entity->get_obj_id()
	});
}

bool game_level::set_level(const std::string& v, bool restart)
{
	if (!restart && !filename.compare(v))
		return false;
	
	filename = v;

	auto slash_pos = filename.find_last_of('\\') + 1;

	name = filename.substr(slash_pos, filename.find_last_of('.') - slash_pos);

	reset_level_data();

	return true;
}

bool game_level::has_player(game_player* player)
{
	return (player ? instances.players.contains(player) : false);
}

bool game_level::is_entity_streamed_by(SYNC_ID entity_sid, game_player* player)
{
	auto entity = g_level->get_entity_by_sid(entity_sid);
	return (entity && entity->is_streamed_by(player));
}

bool game_level::is_entity_streamed_by(game_entity_base* entity, game_player* player)
{
	return (entity && entity->is_streamed_by(player));
}

level_entity_type game_level::get_level_entity_subtype_from_obj_id(int obj_id)
{
	auto it = subtypes.find(obj_id);
	return (it != subtypes.end() ? it->second : ENTITY_LEVEL_TYPE_NONE);
}

SYNC_ID game_level::generate_sync_id()
{
	SYNC_ID sid = 0;

	do sid = 0x10000000 + mt() % (0xEFFFFFF0);
	while (all_entities.contains(sid));

	return sid;
}

game_entity_base* game_level::get_entity_by_sid(SYNC_ID sid)
{
	auto it = all_entities.find(sid);
	return (it != all_entities.end() ? it->second : nullptr);
}

game_player* game_level::get_player_by_id(PLAYER_ID id)
{
	auto it = players.find(id);
	return (it != players.end() ? it->second : nullptr);
}

game_player* game_level::get_random_player(bool joined)
{
	if (instances.players.empty())
		return nullptr;

	std::vector<game_player*> out;

	if (joined)
	{
		std::vector<game_player*> joined_players;

		for (const auto& player : instances.players)
			if (player->get_net()->has_fully_joined())
				joined_players.push_back(player);

		std::sample(joined_players.begin(), joined_players.end(), std::back_inserter(out), 1, std::mt19937_64(std::random_device {} ()));
	}
	else std::sample(instances.players.begin(), instances.players.end(), std::back_inserter(out), 1, std::mt19937_64(std::random_device {} ()));

	return *out.begin();
}
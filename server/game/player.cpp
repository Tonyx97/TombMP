import utils;
import prof;

#include <shared/defs.h>

#include <server/server.h>

#include "player.h"

game_player::game_player(net_player* net, SYNC_ID sid) : net(net)
{
	type = ENTITY_TYPE_PLAYER;
	sync_id = sid;
}

game_player::~game_player()
{
	clear_streamed_entities();
}

void game_player::send_stream_info()
{
	if (!is_spawned())
		return;

	auto bs = g_server->create_packet(ID_SYNC_STREAM_INFO);

	bs->Write(static_cast<int>(added_streamed_entities.size() + removed_streamed_entities.size()));

	for (auto entity : added_streamed_entities)   bs->Write(gns::sync::stream_info { .sid = entity->get_sync_id(), .add = true });
	for (auto entity : removed_streamed_entities) bs->Write(gns::sync::stream_info { .sid = entity->get_sync_id(), .add = false });

	g_server->send_packet_broadcast(bs, net->get_sys_address(), false);

	clear_stream_info();
}

void game_player::clear_stream_info(bool reset_streaming_info)
{
	removed_streamed_entities.clear();
	added_streamed_entities.clear();

	if (reset_streaming_info)
	{
		streaming_info.clear();
		streaming_synced = false;
	}
}

void game_player::clear_streamed_entities()
{
	for (auto entity : streamed_entities)
		entity->reset_streaming_info();

	streamed_entities.clear();

	clear_stream_info();
}

bool game_player::transfer_entity_ownership(game_entity_base* entity, game_player* new_player, int timeout)
{
	if (this == new_player)
	{
		entity->set_streaming_timeout(timeout);
		entity->set_streaming_lock(timeout == -1);

		return true;
	}

	if (!is_spawned())
		return false;

	if (!new_player)
		entity->reset_streaming_info();
	else if (!new_player->acquire_entity_ownership(entity, timeout))
		return false;

	auto bs = g_server->create_packet(ID_SYNC_STREAM_INFO);

	remove_streamed_entity(entity);

	bs->Write(static_cast<int>(removed_streamed_entities.size()));

	for (auto entity : removed_streamed_entities) bs->Write(gns::sync::stream_info { .sid = entity->get_sync_id(), .add = false });

	g_server->send_packet_broadcast(bs, net->get_sys_address(), false);

	clear_stream_info(false);

	return true;
}

bool game_player::acquire_entity_ownership(game_entity_base* entity, int timeout)
{
	if (!is_spawned())
		return false;

	auto bs = g_server->create_packet(ID_SYNC_STREAM_INFO);

	add_streamed_entity(entity);

	entity->set_streamer(this);
	entity->set_streaming_timeout(timeout);
	entity->set_streaming_lock(timeout == -1);

	bs->Write(static_cast<int>(added_streamed_entities.size()));

	for (auto entity : added_streamed_entities) bs->Write(gns::sync::stream_info { .sid = entity->get_sync_id(), .add = true });

	g_server->send_packet_broadcast(bs, net->get_sys_address(), false);

	clear_stream_info(false);

	return true;
}

bool game_player::spawn(const int_vec3& pos, const short_vec3& rot, int16_t hp)
{
	if (spawned)
		return true;
	
	set_position(pos);
	set_rotation(rot);
	set_health(hp);

	gns::player::spawn info
	{
		.name = get_name().c_str(),
		.position = pos,
		.rotation = rot,
		.id = net->get_id(),
		.sid = get_sync_id(),
		.health = hp,
		.set_position = true,
		.set_rotation = true
	};

#ifdef _DEBUG
	prof::printt(PURPLE, "Player {0:#x} spawned ({0:#x})", net->get_id(), sync_id);
#endif

	return (spawned = g_server->send_packet_broadcast(ID_PLAYER_SPAWN, info));
}

bool game_player::spawn(const int_vec3& pos, int16_t hp)
{
	if (spawned)
		return true;

	set_position(pos);
	set_health(hp);

	gns::player::spawn info
	{
		.name = get_name().c_str(),
		.position = pos,
		.id = net->get_id(),
		.sid = get_sync_id(),
		.health = hp,
		.set_position = true,
		.set_rotation = false,
	};

#ifdef _DEBUG
	prof::printt(PURPLE, "Player {0:#x} spawned ({0:#x})", net->get_id(), sync_id);
#endif

	return (spawned = g_server->send_packet_broadcast(ID_PLAYER_SPAWN, info));
}

bool game_player::spawn(int16_t hp)
{
	if (spawned)
		return true;

	set_health(hp);

	gns::player::spawn info
	{
		.name = get_name().c_str(),
		.id = net->get_id(),
		.sid = get_sync_id(),
		.health = hp,
		.set_position = false,
		.set_rotation = false,
	};

#ifdef _DEBUG
	prof::printt(PURPLE, "Player {0:#x} spawned ({0:#x})", net->get_id(), sync_id);
#endif

	return (spawned = g_server->send_packet_broadcast(ID_PLAYER_SPAWN, info));
}

PLAYER_ID game_player::get_id() const
{
	return net->get_id();
}

gns::sync::stream_sync_info* game_player::get_stream_info_for_entity(SYNC_ID sid)
{
	if (auto it = streaming_info.find(sid); it != streaming_info.end())
		return &it->second;
	return nullptr;
}

const std::string& game_player::get_name() const
{
	return net->get_name();
}
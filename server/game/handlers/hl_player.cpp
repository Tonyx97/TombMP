import utils;

#include <shared/defs.h>
#include <shared/scripting/resource.h>
#include <shared/scripting/resource_system.h>
#include <shared/scripting/events.h>

#include <server/server.h>

#include <game/entity_base.h>
#include <game/level.h>
#include <game/player.h>

#include <net/net_player.h>

#include "hl_defs.h"

void player_handlers::handle_packet(net_player* n_player, uint16_t pid)
{
	if (!n_player)
		return;

	switch (pid)
	{
	case ID_PLAYER_SYNC_PLAYERS:	return on_player_sync_with_players(n_player);
	case ID_PLAYER_INFO:			return on_player_info(n_player);
	case ID_PLAYER_DEATH:			return on_player_death(n_player);
	}
}

void player_handlers::on_player_sync_with_players(net_player* n_player)
{
	auto players_count = g_level->get_instanced_players_count();
	if (players_count <= 0)
		return;

	auto bs = g_server->create_packet(ID_PLAYER_SYNC_PLAYERS);
	auto curr_player = n_player->get_player();

	// total info count will be num players minus one
	// because of the player who sent the packet

	bs->Write(players_count - 1);
	bs->Write(curr_player->get_sync_id());

	g_level->for_each_player([&](game_player* player)
	{
		if (player != curr_player)
			bs->Write(gns::player::initial_sync { .name = player->get_name().c_str(), .id = player->get_id(), .sid = player->get_sync_id() });
	});

	g_server->send_packet_broadcast(bs, n_player->get_sys_address(), false);
}

void player_handlers::on_player_info(net_player* n_player)
{
	gns::player::info info; g_server->read_packet_ex(info);

	info.id = n_player->get_id();
	info.ping = n_player->get_ping();

	auto player = n_player->get_player();

	player->set_ping(info.ping);
	player->set_health(info.health);

	g_server->send_packet_broadcast_ex(ID_PLAYER_INFO, n_player->get_sys_address(), true, info);
}

void player_handlers::on_player_death(net_player* n_player)
{
	g_resource->trigger_event(events::player::ON_PLAYER_DIED, n_player->get_player());

	g_server->send_packet_broadcast(ID_PLAYER_DEATH, n_player->get_id());
}
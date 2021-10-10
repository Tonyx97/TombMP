import utils;

#include <shared/defs.h>

#include <server/server.h>

#include <net/net_player.h>

#include "hl_defs.h"

void level_handlers::handle_packet(net_player* n_player, uint16_t pid)
{
	if (!n_player)
		return;

	switch (pid)
	{
	case ID_LEVEL_FLIP_MAP:	return on_level_flip_map(n_player);
	}
}

void level_handlers::on_level_flip_map(net_player* n_player)
{
	g_server->send_packet_broadcast_ex(ID_LEVEL_FLIP_MAP, n_player->get_sys_address(), true);
}
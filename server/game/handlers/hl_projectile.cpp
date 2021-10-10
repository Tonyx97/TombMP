import utils;

#include <shared/defs.h>

#include <server/server.h>

#include <net/net_player.h>

#include "hl_defs.h"

void projectile_handlers::handle_packet(net_player* n_player, uint16_t pid)
{
	if (!n_player)
		return;

	switch (pid)
	{
	case ID_PROJECTILE_CREATE:	return on_projectile_create(n_player);
	}
}

void projectile_handlers::on_projectile_create(net_player* n_player)
{
	gns::projectile::create info; g_server->read_packet_ex(info);

	g_server->send_packet_broadcast_ex(ID_PROJECTILE_CREATE, n_player->get_sys_address(), true, info);
}
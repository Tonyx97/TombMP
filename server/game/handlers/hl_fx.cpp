import utils;

#include <shared/defs.h>

#include <server/server.h>

#include <net/net_player.h>

#include "hl_defs.h"

void fx_handlers::handle_packet(net_player* n_player, uint16_t pid)
{
	if (!n_player)
		return;

	switch (pid)
	{
	case ID_FX_GUN_SMOKE:	return on_fx_gun_smoke(n_player);
	case ID_FX_GUNSHELL:	return on_fx_gunshell(n_player);
	}
}

void fx_handlers::on_fx_gun_smoke(net_player* n_player)
{
	gns::fx::gun_smoke info; g_server->read_packet_ex(info);

	g_server->send_packet_broadcast_ex(ID_FX_GUN_SMOKE, n_player->get_sys_address(), true, info);
}

void fx_handlers::on_fx_gunshell(net_player* n_player)
{
	gns::fx::gunshell info; g_server->read_packet_ex(info);

	g_server->send_packet_broadcast_ex(ID_FX_GUNSHELL, n_player->get_sys_address(), true, info);
}
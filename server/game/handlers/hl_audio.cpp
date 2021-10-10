import utils;

#include <shared/defs.h>

#include <server/server.h>

#include <net/net_player.h>

#include "hl_defs.h"

void audio_handlers::handle_packet(net_player* n_player, uint16_t pid)
{
	if (!n_player)
		return;

	switch (pid)
	{
	case ID_AUDIO_PLAY:	return on_audio_play(n_player);
	case ID_AUDIO_STOP:	return on_audio_stop(n_player);
	}
}

void audio_handlers::on_audio_play(net_player* n_player)
{
	gns::audio::play info; g_server->read_packet_ex(info);

	g_server->send_packet_broadcast_ex(ID_AUDIO_PLAY, n_player->get_sys_address(), true, info);
}

void audio_handlers::on_audio_stop(net_player* n_player)
{
	gns::audio::stop info; g_server->read_packet_ex(info);

	g_server->send_packet_broadcast_ex(ID_AUDIO_STOP, n_player->get_sys_address(), true, info);
}
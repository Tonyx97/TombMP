import utils;
import prof;

#include <shared/defs.h>
#include <shared/scripting/resource_system.h>
#include <shared/scripting/events.h>

#include <scripting/debugger.h>

#include <mp/client.h>
#include <mp/chat.h>
#include <mp/game/player.h>
#include <mp/game/level.h>

#include "hl_net_defs.h"

void net_player_handlers::handle_packet(uint16_t pid)
{
	switch (pid)
	{
	case ID_NET_PLAYER_CONNECT:			return on_player_connect();
	case ID_NET_PLAYER_NAME:			return on_player_name();
	case ID_NET_PLAYER_JOIN:			return on_player_join();
	case ID_NET_PLAYER_QUIT:			return on_player_quit();
	case ID_NET_PLAYER_CHAT:			return on_player_chat();
	case ID_NET_PLAYER_NOTIFICATION:	return on_player_notification();
	case ID_NET_PLAYER_SCRIPT_ERROR:	return on_player_script_error();
	}
}

void net_player_handlers::on_player_connect()
{
	// we shouldn't create the game_player if we didn't join yet (synced all resources)
	
	if (!g_client->is_ready())
		return;

	gns::net_player::connect info; g_client->read_packet_ex(info);

	if (!g_level->add_player(info.id, info.sid))
		prof::critical_error("Recently joined player couldn't be created");
}

void net_player_handlers::on_player_name()
{
	gns::net_player::name_bc info; g_client->read_packet_ex(info);

	g_chat->add_chat_msg(L"##FFA530FF" + utils::string::convert(*info.old_name) + L" is now " + utils::string::convert(*info.new_name));

	prof::print(YELLOW, "'{}' changed their name to '{}'", *info.old_name, *info.new_name);
}

void net_player_handlers::on_player_join()
{
	gns::net_player::join info; g_client->read_packet_ex(info);

	g_chat->add_chat_msg(L"##00FF00FF" + utils::string::convert(*info.name) + L" has joined the server");

	prof::print(YELLOW, "'{}' joined the server", *info.name);

	g_resource->trigger_event(events::player::ON_PLAYER_JOIN, *info.name);
}

void net_player_handlers::on_player_quit()
{
	gns::net_player::quit info; g_client->read_packet_ex(info);

	g_chat->add_chat_msg(L"##FF0000FF" + utils::string::convert(*info.name) + L" left the server (" + (info.timed_out ? L"Connection lost" : L"Disconnected") + L")");

	prof::print(YELLOW, "'{}' left the server", *info.name);

	if (auto player = g_level->get_player_by_id(info.id))
	{
		g_resource->trigger_event(events::player::ON_PLAYER_QUIT, player);
		g_level->remove_entity(player);
	}
}

void net_player_handlers::on_player_chat()
{
	gns::net_player::chat info; g_client->read_packet_ex(info);

	g_chat->add_chat_msg(L"##FFA500FF" + utils::string::convert(*info.player_name) + L": ##FFFFFFFF" + *info.message);
}

void net_player_handlers::on_player_notification()
{
	gns::net_player::notification info; g_client->read_packet_ex(info);

	g_chat->add_chat_msg(utils::string::color_to_string(info.color) + utils::string::convert(*info.text));
}

void net_player_handlers::on_player_script_error()
{
	gns::net_player::script_error info; g_client->read_packet_ex(info);

	g_debugger->add_server_msg(*info.error);
}
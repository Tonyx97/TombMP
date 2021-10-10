import utils;
import prof;

#include <shared/defs.h>
#include <shared/scripting/resource.h>
#include <shared/scripting/resource_system.h>
#include <shared/scripting/events.h>

#include <server/server.h>

#include <game/player.h>

#include <net/net_player.h>

#include "hl_net_defs.h"

void net_player_handlers::handle_packet(net_player* player, uint16_t pid)
{
	if (!player)
		return;

	switch (pid)
	{
	case ID_DISCONNECTION_NOTIFICATION:
	case ID_CONNECTION_LOST:			return on_player_quit(player, pid == ID_CONNECTION_LOST);
	case ID_NET_PLAYER_JOIN:			return on_player_join(player);
	case ID_NET_PLAYER_NAME:			return on_player_name(player);
	case ID_NET_PLAYER_CHAT:			return on_player_chat(player);
	case ID_NET_PLAYER_REGISTER:		return on_player_register(player);
	case ID_NET_PLAYER_LOGIN:			return on_player_login(player);
	case ID_NET_PLAYER_LOGOUT:			return on_player_logout(player);
	case ID_NET_PLAYER_NET_FLAGS:		return on_player_net_flags(player);
	}
}

void net_player_handlers::on_player_join(net_player* player)
{
	gns::net_player::join info; g_server->read_packet_ex(info);

	info.id = player->get_id();

	if (!info.ready)
	{
		player->set_name(*info.name);

		g_server->send_packet_broadcast_ex(ID_NET_PLAYER_JOIN, player->get_sys_address(), true, info);

		prof::printt(WHITE, "'{}' joined the server", *info.name);
	}
	else
	{
		player->set_fully_joined();
		player->sync_all_resources_status();
	}

	g_resource->trigger_event(events::player::ON_PLAYER_JOIN, player->get_player(), info.ready);
}

void net_player_handlers::on_player_quit(net_player* player, bool timeout)
{
	prof::printt(WHITE, "'{}' left the server", player->get_name().c_str());

	gns::net_player::quit out_info {};

	out_info.name = player->get_name().c_str();
	out_info.id = player->get_id();
	out_info.timed_out = timeout;

	g_resource->trigger_event(events::player::ON_PLAYER_QUIT, player->get_player());

	g_server->send_packet_broadcast_ex(ID_NET_PLAYER_QUIT, player->get_sys_address(), true, out_info);
	g_server->remove_player(player);
}

void net_player_handlers::on_player_name(net_player* player)
{
	gns::net_player::join info; g_server->read_packet_ex(info);

	gns::net_player::name_bc out_info {};

	out_info.old_name = player->get_name().c_str();
	out_info.new_name = info.name;

	player->set_name(*info.name);

	g_server->send_packet_broadcast(ID_NET_PLAYER_NAME, out_info);

	prof::printt(WHITE, "'{}' changed their name to '{}'", player->get_name().c_str(), *info.name);
}

void net_player_handlers::on_player_chat(net_player* player)
{
	gns::net_player::chat info; g_server->read_packet_ex(info);

	g_server->send_packet_broadcast(ID_NET_PLAYER_CHAT, info);

	prof::printt(WHITE, "'{}': {}", *info.player_name, utils::string::convert(*info.message).c_str());
}

void net_player_handlers::on_player_register(net_player* player)
{
	gns::net_player::register_login info; g_server->read_packet_ex(info);

	if (!player->is_logged_in())
	{
		if (g_server->register_user(*info.user, *info.pass))
			player->send_notification("Successfully registered", 0x00FF00FF);
		else player->send_notification("User already exists", 0xFF0000FF);
	}
	else player->send_notification("You are already logged in", 0xFF0000FF);
}

void net_player_handlers::on_player_login(net_player* player)
{
	gns::net_player::register_login info; g_server->read_packet_ex(info);

	if (!player->is_logged_in())
	{
		if (!g_server->is_user_logged_in(*info.user))
		{
			bool invalid_pass = false;

			if (player->login(*info.user, *info.pass, invalid_pass))
			{
				std::string text;

				if (player->is_owner())			 text = "Successfully logged in (as owner)";
				else if (player->is_admin())	 text = "Successfully logged in (as admin)";
				else if (player->is_moderator()) text = "Successfully logged in (as moderator)";
				else							 text = "Successfully logged in";

				player->send_notification(text, 0x00FF00FF);
			}
			else player->send_notification(invalid_pass ? "Invalid password" : "User does not exist", 0xFF0000FF);
		}
		else player->send_notification("User already logged in", 0xFF0000FF);
	}
	else player->send_notification("You are already logged in", 0xFF0000FF);
}

void net_player_handlers::on_player_logout(net_player* player)
{
	if (player->logout())
		player->send_notification("Successfully logged out", 0xFFFF00FF);
	else player->send_notification("You are not logged in", 0xFF0000FF);
}

void net_player_handlers::on_player_net_flags(net_player* player)
{
	gns::net_player::net_flags info; g_server->read_packet_ex(info);

	if (player->is_owner())
	{
		if (g_server->set_user_flags(*info.user, info.flags))
			player->send_notification("Success", 0xFFFF00FF);
		else player->send_notification("User does not exist", 0xFF0000FF);
	}
	else player->send_notification("Not allowed", 0xFF0000FF);
}
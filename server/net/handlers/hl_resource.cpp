import utils;
import prof;

#include <shared/defs.h>
#include <shared/scripting/resource_system.h>
#include <shared/scripting/resource.h>

#include <server/server.h>

#include <net/net_player.h>

#include "hl_net_defs.h"

void resource_handlers::handle_packet(net_player* player, uint16_t pid)
{
	if (!player)
		return;

	switch (pid)
	{
	case ID_RESOURCE_SYNC_ALL:			return on_resource_sync_all(player);
	case ID_RESOURCE_SYNC_ALL_STATUS:	return on_resource_sync_all_status(player);
	case ID_RESOURCE_ACTION:
	{
		gns::resource::action info; g_server->read_packet_ex(info);

		switch (info.action)
		{
		case RESOURCE_ACTION_START:		return on_resource_start(player, *info.name);
		case RESOURCE_ACTION_STOP:		return on_resource_stop(player, *info.name);
		case RESOURCE_ACTION_RESTART:	return on_resource_restart(player, *info.name);
		}
	}
	case ID_RESOURCE_REFRESH_ALL:		return on_resource_refresh(player);
	}
}

void resource_handlers::on_resource_sync_all(net_player* player)
{
	auto bs = g_server->get_current_bs();

	if (int size; bs->Read(size) && size >= 0 && size <= resource_system::MAX_RESOURCES)
	{
		for (int i = 0; i < size; ++i)
		{
			gns::resource::info info; bs->Read(info);

			prof::print(RED, "Received info '{}' | '{}' | {:#x}", *info.rsrc, *info.name, info.hash);

			player->add_file_to_check(info);
		}
	}

	player->compare_and_send_files();
}

void resource_handlers::on_resource_sync_all_status(net_player* player)
{
	if (player->has_fully_joined())
		player->sync_all_resources_status();
}

void resource_handlers::on_resource_start(net_player* player, const std::string& name)
{
	if (player->has_resource_permissions())
	{
		switch (g_resource->start_resource(name))
		{
		case SCRIPT_ACTION_FAIL:
		case SCRIPT_ACTION_SCRIPT_ERROR:	player->send_notification(g_resource->get_last_error(), 0xFF0000FF); break;
		case SCRIPT_ACTION_ALREADY_RUNNING: player->send_notification("Resource already running", 0xFF0000FF);	 break;
		case SCRIPT_ACTION_NOT_FOUND:		player->send_notification("Resource does not exist", 0xFF0000FF);	 break;
		case SCRIPT_ACTION_OK:				player->send_notification("Resource started", 0x00FF00FF);			 break;
		}
	}
	else player->send_notification("Not allowed", 0xFF0000FF);
}

void resource_handlers::on_resource_stop(net_player* player, const std::string& name)
{
	if (player->has_resource_permissions())
	{
		switch (g_resource->stop_resource(name))
		{
		case SCRIPT_ACTION_NOT_RUNNING:		player->send_notification("Resource not running", 0xFF0000FF);			break;
		case SCRIPT_ACTION_NOT_FOUND:		player->send_notification("Resource does not exist", 0xFF0000FF);		break;
		case SCRIPT_ACTION_OK:				player->send_notification("Resource stopped", 0x00FF00FF);				break;
		}
	}
	else player->send_notification("Not allowed", 0xFF0000FF);
}

void resource_handlers::on_resource_restart(net_player* player, const std::string& name)
{
	if (player->has_resource_permissions())
	{
		switch (g_resource->restart_resource(name))
		{
		case SCRIPT_ACTION_ALREADY_RUNNING: player->send_notification("Resource already running", 0xFF0000FF);	 break;
		case SCRIPT_ACTION_NOT_RUNNING:		player->send_notification("Resource not running", 0xFF0000FF);		 break;
		case SCRIPT_ACTION_SCRIPT_ERROR:	player->send_notification(g_resource->get_last_error(), 0xFF0000FF); break;
		case SCRIPT_ACTION_NOT_FOUND:		player->send_notification("Resource does not exist", 0xFF0000FF);	 break;
		case SCRIPT_ACTION_OK:				player->send_notification("Resource restarted", 0x00FF00FF);		 break;
		}
	}
	else player->send_notification("Not allowed", 0xFF0000FF);
}

void resource_handlers::on_resource_refresh(net_player* player)
{
	if (player->has_resource_permissions())
		g_server->load_resources();
	else player->send_notification("Not allowed", 0xFF0000FF);
}
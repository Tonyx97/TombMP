import utils;
import prof;

#include <shared/defs.h>
#include <shared/scripting/resource_system.h>
#include <shared/scripting/resource.h>

#include <scripting/events.h>

#include <mp/client.h>

#include "hl_net_defs.h"

void resource_handlers::handle_packet(uint16_t pid)
{
	switch (pid)
	{
	case ID_RESOURCE_SYNC_ALL:			return on_resource_sync_all();
	case ID_RESOURCE_SYNC_ALL_STATUS:	return on_resource_sync_all_status();
	case ID_RESOURCE_ACTION:
	{
		/*gns::resource::action info; g_client->read_packet_ex(info);

		switch (info.action)
		{
		case RESOURCE_ACTION_STOP: return on_resource_stop(*info.name);
		}*/
	}
	}
}

void resource_handlers::on_resource_sync_all()
{
	g_client->sync_resource_all();

	prof::print(YELLOW, "Syncing all resources...");
}

void resource_handlers::on_resource_sync_all_status()
{
	auto bs = g_client->get_current_bs();

	int size = 0; bs->Read(size);
	if (size <= 0)
		return;
	
	for (int i = 0; i < size; ++i)
	{
		gns::resource::action info; bs->Read(info);

		std::string rsrc_name = *info.name;

		bool already_updated = false;

		if (!g_resource->get_resource(rsrc_name))
			if (!(already_updated = g_resource->load_client_resource(rsrc_name, g_client->get_rsrc_file_list(rsrc_name))))
				prof::critical_error("Resource {} could not be loaded", rsrc_name);

		switch (info.action)
		{
		case RESOURCE_STATUS_STARTED:
		{
			if (!already_updated && !g_resource->load_client_resource(rsrc_name, g_client->get_rsrc_file_list(rsrc_name)))
				prof::critical_error("Resource {} could not be updated", rsrc_name);

			g_resource->start_resource(rsrc_name);

			break;
		}
		case RESOURCE_STATUS_STOPPED: g_resource->stop_resource(rsrc_name); break;
		}

		prof::print(YELLOW, "Resource {} sync status: {}", *info.name, info.action);
	}

	if (g_client->level_was_loaded())
	{
		g_resource->trigger_event(events::level::ON_LEVEL_LOAD);
		g_client->set_level_loaded(false);
	}

	g_client->clear_rsrcs_file_list();
}

void resource_handlers::on_resource_stop(const std::string& name)
{
	g_resource->stop_resource(name);
}
import utils;

#include <shared/defs.h>
#include <shared/scripting/resource_system.h>

#include <mp/client.h>
#include <mp/game/level.h>

#include <game/control.h>

#include "hl_defs.h"

void level_handlers::handle_packet(uint16_t pid)
{
	switch (pid)
	{
	case ID_LEVEL_LOAD:			return on_level_load();
	case ID_LEVEL_FLIP_MAP:		return on_level_flip_map();
	}
}

void level_handlers::on_level_load()
{
	gns::level::load info; g_client->read_packet_ex(info);

	g_level->set_level(*info.name, info.restart);
}

void level_handlers::on_level_flip_map()
{
	FlipMap(false);
}
#include <shared/defs.h>
#include <shared/scripting/globals.h>
#include <shared/scripting/script.h>
#include <shared/scripting/resource_system.h>

#include <server/server.h>

#include <game/level.h>

#include "sf_defs.h"

void sf_level::register_functions(sol::state* vm)
{
	vm->set_function("setCurrentLevel", [&](sol::this_state state, const char* filename, bool restart)
	{
		auto level_filename = resource_system::RESOURCES_PATH + script::get_global_string(state, scripting::globals::RESOURCE_NAME) + '\\' + filename;

		if (!std::filesystem::is_regular_file(level_filename))
			return false;

		if (g_level->set_level(level_filename, restart))
		{
			gns::level::load info;

			info.name = level_filename.c_str();
			info.restart = restart;

			g_server->send_packet_broadcast(ID_LEVEL_LOAD, info);
		}

		return true;
	});
}
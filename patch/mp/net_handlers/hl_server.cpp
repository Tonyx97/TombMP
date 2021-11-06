import utils;

#include <shared/defs.h>
#include <shared/scripting/resource_system.h>
#include <shared/scripting/resource.h>

#include <mp/client.h>
#include <mp/game/player.h>
#include <mp/game/level.h>

#include "hl_net_defs.h"

void server_handlers::handle_packet(uint16_t pid)
{
	switch (pid)
	{
	case ID_SERVER_GAME_SETTINGS:			return on_game_settings();
	case ID_SERVER_TRIGGER_CLIENT_EVENT:	return on_trigger_client_event();
	}
}

void server_handlers::on_game_settings()
{
	g_client->read_packet_ex(g_client->get_game_settings());
}

void server_handlers::on_trigger_client_event()
{
	auto bs = g_client->get_current_bs();

	int size = 0;

	bs->Read(size);

	std::vector<std::any> args;

	for (int i = 0; i < size; ++i)
	{
		sol::type type;

		bs->Read(type);

		switch (type)
		{
		case sol::type::userdata:
		{
			int ud_type; bs->Read(ud_type);

			switch (ud_type)
			{
			case ENTITY_TYPE_PLAYER:
			{
				PLAYER_ID player_id; bs->Read(player_id);

				if (auto player = g_level->get_player_by_id(player_id))
					args.push_back(player);

				break;
			}
			case ENTITY_TYPE_LEVEL:
			{
				SYNC_ID sid; bs->Read(sid);

				if (auto entity = reinterpret_cast<game_entity*>(g_level->get_entity_by_sid(sid)))
					args.push_back(entity);

				break;
			}
			}

			break;
		}
		case sol::type::string:
		{
			SLNet::RakString v;

			bs->Read(v);

			args.push_back(std::string(v.C_String()));

			break;
		}
		case sol::type::number:
		{
			bool is_int;

			bs->Read(is_int);

			if (is_int)
			{
				int64_t v;

				bs->Read(v);

				args.push_back(v);
			}
			else
			{
				double v;

				bs->Read(v);

				args.push_back(v);
			}

			break;
		}
		case sol::type::boolean:
		{
			bool v;

			bs->Read(v);

			args.push_back(v);
		}
		}
	}

	gns::server::trigger_event info; g_client->read_packet_ex(info);

	g_resource->trigger_remote_event(*info.name, args);
}
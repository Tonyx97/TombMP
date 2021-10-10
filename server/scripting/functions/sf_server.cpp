#include <shared/defs.h>
#include <shared/scripting/globals.h>
#include <shared/scripting/script.h>
#include <shared/scripting/resource_system.h>

#include <net/net_player.h>

#include <game/player.h>
#include <game/level.h>

#include <server/server.h>

#include "sf_defs.h"

void sf_server::register_functions(sol::state* vm)
{
	vm->set_function("triggerClientEvent", [&](const char* name, sol::variadic_args va)
	{
		int args_size = va.size();
		if (args_size <= 0)
			return false;

		const auto& first_arg = va[0];

		auto send_packet_data = [&](game_player* player = nullptr)
		{
			auto bs = g_server->create_packet(ID_SERVER_TRIGGER_CLIENT_EVENT);

			bs->Write(args_size - 1);

			for (int i = 1; i < args_size; ++i)
			{
				const auto& v = va[i];

				switch (const auto type = v.get_type())
				{
				case sol::type::userdata:
				{
					bs->Write(type);

					if (v.is<game_player*>())
					{
						if (auto player = v.as<game_player*>(); g_level->has_player(player))
						{
							bs->Write(int(player->get_type()));
							bs->Write(player->get_id());
						}
					}

					break;
				}
				case sol::type::string:
					bs->Write(type);
					bs->Write(SLNet::RakString(v.as<const char*>()));
					break;
				case sol::type::number:
				{
					const bool is_int = v.is<int64_t>();

					bs->Write(type);
					bs->Write(is_int);

					if (is_int) bs->Write(v.as<int64_t>());
					else		bs->Write(v.as<double>());

					break;
				}
				case sol::type::boolean:
					bs->Write(type);
					bs->Write(v.as<bool>());
				}
			}

			gns::server::trigger_event info {};

			info.name = name;

			if (!player)
				return g_server->send_packet(bs, info);
			else return g_server->send_packet_broadcast(bs, player->get_net()->get_sys_address(), false, info);
		};

		switch (first_arg.get_type())
		{
		case sol::type::userdata:
		{
			if (!first_arg.is<game_player*>())
				return false;

			auto target = first_arg.as<game_player*>();

			if (!g_level->has_player(target))
				return false;

			return send_packet_data(target);
		}
		case sol::type::nil: return send_packet_data();
		/*case sol::type::table:
		{
			auto target_table = first_arg.as<sol::table>();

			for (const auto& [k, v] : target_table)
				if (!v.is<game_player*>())
					return false;
				else
				{
					auto target = v.as<game_player*>();

					PRINT(DBG_GREEN, "Sending to '%s'", target->get_name().c_str());
				}

			return true;
		}*/
		}

		return false;
	});

	vm->set_function("setServerGameSettingEnabled", [&](const char* name, bool enabled)
	{
		g_server->set_game_setting_enabled(game_setting(utils::hash::JENKINS(name)), enabled);
	});

	vm->set_function("isServerGameSettingEnabled", [&](const char* name)
	{
		return g_server->is_game_setting_enabled(game_setting(utils::hash::JENKINS(name)));
	});
}
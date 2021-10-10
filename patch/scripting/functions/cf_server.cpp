#include <shared/defs.h>

#include <sol/sol.hpp>

#include <shared/scripting/script.h>

#include <mp/client.h>
#include <mp/game/player.h>
#include <mp/game/level.h>

#include "cf_defs.h"

void cf_server::register_functions(sol::state* vm)
{
	vm->set_function("triggerServerEvent", [&](const char* name, sol::variadic_args va)
	{
		auto bs = g_client->create_packet(ID_SERVER_TRIGGER_SERVER_EVENT);

		const int args_size = va.size();

		bs->Write(args_size);

		for (int i = 0; i < args_size; ++i)
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

		return g_client->send_packet(bs, info);
	});
}
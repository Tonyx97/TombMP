#include <shared/defs.h>
#include <shared/scripting/script.h>

#include <server/server.h>

#include <game/level.h>
#include <game/player.h>

#include "sf_defs.h"

void sf_player::register_functions(sol::state* vm)
{
	vm->set_function("spawnPlayer", [&](game_player* player, sol::variadic_args va)
	{
		if (!g_level->has_player(player))
			return false;

		auto args_size = va.size();
		
		if (args_size == 0)
			return player->spawn(1000);
		else if (args_size == 1)
			return player->spawn(va[0].as<int16_t>());
		else if (args_size >= 3 && args_size <= 7)
		{
			int x = va[0].as<int>(),
				y = va[1].as<int>(),
				z = va[2].as<int>();

			if (args_size == 3)
				return player->spawn({ x, y, z });
			else if (args_size == 4)
				return player->spawn({ x, y, z }, va[3].as<int16_t>());
			else if (args_size >= 6)
			{
				auto rx = va[3].as<int16_t>(),
				 	 ry = va[4].as<int16_t>(),
					 rz = va[5].as<int16_t>();

				if (args_size == 6)
					return player->spawn({ x, y, z }, { rx, ry, rz });
				else if (args_size == 7)
					return player->spawn({ x, y, z }, { rx, ry, rz }, va[6].as<int16_t>());
			}
		}

		return false;
	});

	vm->set_function("respawnPlayer", [&](game_player* player, sol::variadic_args va)
	{
		if (!g_level->has_player(player))
			return false;

		auto player_net = player->get_net();

		gns::player::respawn info
		{
			.rotation = { 0, 0, 0 },
			.id = player_net->get_id(),
			.sid = player->get_sync_id(),
			.room = -1,
			.health = 1000,
		};

		int x = 0,
			y = 0,
			z = 0;

		if (va.size() >= 4)
		{
			info.position.x = va[0].is<int>() ? va[0].as<int>() : 0;
			info.position.y = va[1].is<int>() ? va[1].as<int>() : 0;
			info.position.z = va[2].is<int>() ? va[2].as<int>() : 0;
			info.room = va[3].is<int>() ? va[3].as<int>() : -1;

			if (va.size() >= 7)
			{
				info.rotation.x = int16_t(va[5].is<int>() ? va[5].as<int>() : 0);
				info.rotation.y = int16_t(va[6].is<int>() ? va[6].as<int>() : 0);
				info.rotation.z = int16_t(va[7].is<int>() ? va[7].as<int>() : 0);

				if (va.size() == 8)
					info.health = int16_t(va[4].is<int>() ? va[4].as<int>() : 1000);
			}
		}

		info.set_position = (info.room != -1);

		return g_server->send_packet_broadcast_ex(ID_PLAYER_RESPAWN, player_net->get_sys_address(), false, info);
	});

	vm->set_function("getPlayerFromID", [&](PLAYER_ID id)
	{
		return g_level->get_player_by_id(id);
	});

	vm->set_function("isPlayerSpawned", [&](game_player* player) { return player ? player->is_spawned() : false; });
	vm->set_function("getPlayerName", [&](game_player* player) { return player ? player->get_name() : ""; });
	vm->set_function("getPlayerPing", [&](game_player* player) { return player ? player->get_ping() : -1; });
	vm->set_function("getPlayerHealth", [&](game_player* player) { return player ? player->get_health() : -1; });
	vm->set_function("getPlayerRoom", [&](game_player* player) { return player ? player->get_room() : 255; });
	vm->set_function("isPlayerDead", [&](game_player* player) { return player ? player->get_health() <= 0 : -1; });

	vm->set_function("getPlayerFromName", [&](const char* v, bool partial) -> game_player*
	{
		if (!v)
			return nullptr;

		auto search_pred = [&](char x, char y) { return std::tolower(x) == std::tolower(y); };

		auto players = g_level->get_instanced_players();
		auto name = std::string(v);

		for (const auto& player : players)
			if (auto player_name = player->get_name(); std::equal(player_name.begin(), player_name.end(), name.begin(), name.end(), search_pred))
				return player;
			else if (partial)
			{
				if (auto it = std::search(player_name.begin(), player_name.end(), name.begin(), name.end(), search_pred); it != player_name.end())
					return player;
			}

		return nullptr;
	});

	vm->set_function("getPlayerID", [&](game_player* player)
	{
		return player->get_id();
	});

	vm->set_function("isPlayer", [&](game_player* player)
	{
		return g_level->has_player(player);
	});

	vm->set_function("getPlayerPosition", [&](game_player* player) -> std::tuple<int, int, int>
	{
		if (!g_level->has_player(player))
			return { 0, 0, 0 };

		const auto& p = player->get_position();

		return { p.x, p.y, p.z };
	});
}
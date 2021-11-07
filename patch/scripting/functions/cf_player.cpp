import prof;

#include <shared/defs.h>

#include <sol/sol.hpp>

#include <specific/standard.h>
#include <specific/output.h>
#include <specific/global.h>

#include <game/camera.h>
#include <game/laraanim.h>
#include <game/lara2gun.h>
#include <game/invfunc.h>

#include <mp/client.h>
#include <mp/game/level.h>
#include <mp/game/player.h>

#include "cf_defs.h"

void cf_player::register_functions(sol::state* vm)
{
	vm->set_function("setLocalPlayerOnFire", [&](bool v)
	{
		if (v)
			LaraBurn();
		else lara.burn = v;
	});

	vm->set_function("isLocalPlayerOnFire",	[&]() { return !!lara.burn; });
	vm->set_function("setLocalFireColor", [&](const sol::variadic_args& args)
	{
		if (args.size() == 3)
		{
			if (args[0].is<int>() &&
				args[1].is<int>() &&
				args[2].is<int>())
			{
				lara.burn_red = args[0].as<int>();
				lara.burn_green = args[1].as<int>();
				lara.burn_blue = args[2].as<int>();
			}
		}
		else
		{
			lara.burn_red = 255;
			lara.burn_green = 100;
			lara.burn_blue = 48;
		}
	});

	vm->set_function("setLocalPlayerElectric", [&](bool v)	{ lara.electric = v; });
	vm->set_function("isLocalPlayerElectric",  [&]()		{ return !!lara.electric; });

	vm->set_function("setLocalPlayerAir", [&](int16_t v)	{ lara.air = v; });
	vm->set_function("getLocalPlayerAir", [&]()				{ return lara.air; });
	vm->set_function("getMaxAir",		  [&]()				{ return LARA_AIR; });

	vm->set_function("setLocalPlayerExposure", [&](int16_t v)	{ lara.exposure = v; });
	vm->set_function("getLocalPlayerExposure", [&]()			{ return lara.exposure; });
	vm->set_function("getMaxExposure",		   [&]()			{ return LARA_EXPOSURE_TIME; });

	vm->set_function("setLocalPlayerSprint", [&](int16_t v)	{ lara.dash = v; });
	vm->set_function("getLocalPlayerSprint", [&]()			{ return lara.dash; });
	vm->set_function("getMaxSprint",		 [&]()			{ return LARA_DASH_TIME; });
	
	vm->set_function("setLocalPlayerFrozen", [&](bool v) { lara.frozen = v; });
	vm->set_function("isLocalPlayerFrozen", [&]()		 { return lara.frozen; });

	vm->set_function("setLocalPlayerFlags", [&](int32_t v)    { game_level::LOCALPLAYER()->set_entity_flags(v); });
	vm->set_function("addLocalPlayerFlags", [&](int32_t v)	  { game_level::LOCALPLAYER()->add_entity_flags(v); });
	vm->set_function("removeLocalPlayerFlags", [&](int32_t v) { game_level::LOCALPLAYER()->remove_entity_flags(v); });
	vm->set_function("getLocalPlayerFlags", [&]()			  { return game_level::LOCALPLAYER()->get_entity_flags(); });

	vm->set_function("getPlayerItem", [&](game_player* player) { return player->get_item(); });

	vm->set_function("warpPlayer", [&](int x, int y, int z, int room, bool reset_anims)
	{
		if (reset_anims)
			ResetLaraState();

		lara_item->pos = { x, y, z, 0, 0, 0 };

		ItemNewRoom(lara.item_number, room);
		InitialiseResetCamera();
	});

	vm->set_function("getPlayerFromID", [&](PLAYER_ID id)
	{
		return g_level->get_player_by_id(id);
	});

	vm->set_function("getPlayerFromItem", [&](ITEM_INFO* item)
	{
		return g_level->get_player_by_item(item);
	});

	vm->set_function("getPlayerName",	[&](game_player* player) { return player->get_name(); });
	vm->set_function("getPlayerPing",	[&](game_player* player) { return player->get_ping(); });
	vm->set_function("getPlayerHealth", [&](game_player* player) { return player->get_health(); });
	vm->set_function("isPlayerDead",	[&](game_player* player) { return player->get_health() <= 0; });

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

	vm->set_function("getPlayerHeadRotation", [&](game_player* player) -> std::tuple<PHD_ANGLE, PHD_ANGLE, PHD_ANGLE>
	{
		const auto& rot = player->get_head_rotation();
		return { rot.x, rot.y, rot.z };
	});

	vm->set_function("getPlayerTorsoRotation", [&](game_player* player) -> std::tuple<PHD_ANGLE, PHD_ANGLE, PHD_ANGLE>
	{
		const auto& rot = player->get_torso_rotation();
		return { rot.x, rot.y, rot.z };
	});

	vm->set_function("getPlayerID", [&](game_player* player)
	{
		return player->get_id();
	});

	vm->set_function("addInventoryItem", [&](int id, int amount)
	{
		for (int i = 0; i < amount; ++i)
			if (!Inv_AddItem(id))
				return false;

		return true;
	});

	vm->set_function("removeInventoryItem", [&](int id, int amount)
	{
		for (int i = 0; i < amount; ++i)
			if (!Inv_RemoveItem(id))
				return false;

		return true;
	});

	vm->set_function("removeAllItems", [&]()
	{
		Inv_RemoveAllItems();
	});
}
#include <shared/defs.h>

#include <sol/sol.hpp>

#include <mp/client.h>
#include <mp/game/entity.h>
#include <mp/game/player.h>
#include <mp/game/level.h>

#include "cf_defs.h"

void cf_entity::register_functions(sol::state* vm)
{
	vm->set_function("getPlayersTable", [&]()
	{
		return sol::as_table(g_level->get_instanced_players());
	});

	vm->set_function("getItemsTable", [&]()
	{
		return sol::as_table(g_level->get_instanced_level_entities());
	});

	vm->set_function("getItemFromEntity", [&](game_entity* entity) { return entity->get_item(); });
	vm->set_function("asEntity", [&](game_entity_base* base) { return utils::rtti::safe_cast<game_entity>(base); });
	vm->set_function("isEntity", [&](game_entity* entity) { return g_level->has_entity(entity); });
	vm->set_function("asPlayer", [&](game_entity_base* base) { return utils::rtti::safe_cast<game_player>(base); });
	vm->set_function("isPlayer", [&](game_player* player) { return g_level->has_player(player); });
	vm->set_function("destroyEntity", [&](game_entity_base* entity) { KillItem(entity->get_item_id()); });
	vm->set_function("destroyEntity", [&](ITEM_INFO* item)
	{
		if (auto entity = g_level->get_entity_by_item(item))
			KillItem(entity->get_item_id());
	});
}
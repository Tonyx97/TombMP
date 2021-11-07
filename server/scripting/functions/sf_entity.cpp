#include <shared/defs.h>
#include <shared/scripting/globals.h>
#include <shared/scripting/script.h>
#include <shared/scripting/resource_system.h>

#include <server/server.h>

#include <game/entity.h>
#include <game/player.h>
#include <game/level.h>
#include <game/entity.h>

#include "sf_defs.h"

void sf_entity::register_functions(sol::state* vm)
{
	vm->set_function("getPlayersTable", [&]()
	{
		return sol::as_table(g_level->get_instanced_players());
	});

	vm->set_function("getItemsTable", [&]()
	{
		return sol::as_table(g_level->get_instanced_level_entities());
	});

	vm->set_function("getPlayerSpawns", [&]()
	{
		return sol::as_table(g_level->get_player_spawns());
	});

	vm->set_function("getRandomPlayer", [&](bool joined)
	{
		return g_level->get_random_player(joined);
	});

	vm->set_function("asPlayer", [&](game_entity_base* base) { return utils::rtti::safe_cast<game_player>(base); });
	vm->set_function("isEntity", [&](game_entity* entity) { return g_level->has_entity(entity); });

	vm->set_function("asEntity", [&](game_entity_base* base) { return utils::rtti::safe_cast<game_entity>(base); });
	vm->set_function("isEntity", [&](game_entity* entity) { return g_level->has_entity(entity); });

	vm->set_function("destroyEntity", [&](game_entity* entity) { return g_level->destroy_entity(entity); });

	vm->set_function("spawnEntity", [&](int16_t obj_id, int x, int y, int z, int16_t room, int16_t rx, int16_t ry, int16_t rz) -> game_entity*
	{
		if (!obj_id)
			return nullptr;

		return g_level->spawn_level_entity(obj_id,
		{
			.pos = { x, y, z },
			.rot = { rx, ry, rz },
			.room = room
		});
	});

	vm->set_function("attachEntities", [&](game_entity* a, game_entity* b, int x, int y, int z, int16_t rx, int16_t ry, int16_t rz)
	{
		gns::entity::attach info
		{
			.a_sid = a->get_sync_id(),
			.b_sid = b->get_sync_id(),
			.local_pos = { x, y, z },
			.local_rot = { rx, ry, rz }
		};

		g_level->add_attachment(a, b, info.local_pos, info.local_rot);

		g_server->send_packet_broadcast(ID_ENTITY_ATTACH, info);
	});

	vm->set_function("getVectorInfo", [&](const game_vec3d& v) -> std::tuple<int, int, int, int16_t, int16_t, int16_t, int16_t>
	{
		return { v.pos.x, v.pos.y, v.pos.z, v.rot.x, v.rot.y, v.rot.z, v.room };
	});
}
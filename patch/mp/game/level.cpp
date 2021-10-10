import prof;

#include <shared/defs.h>
#include <shared/scripting/resource.h>
#include <shared/scripting/resource_system.h>

#include <scripting/globals.h>

#include <mp/client.h>

#include "entity.h"
#include "player.h"
#include "level.h"

game_level::~game_level()
{
	clear();
}

game_entity_base* game_level::add_entity_base(game_entity_base* base)
{
	bool created = false,
		 add_to_all_entities_list = false;

	switch (base->get_type())
	{
	case ENTITY_TYPE_PLAYER:
	{
		if (auto player = utils::rtti::safe_cast<game_player>(base))
		{
			if (!player->is_local())
			{
				if (auto player_item = player->get_item())
				{
					entities_by_item.insert({ player_item, player });

					add_to_all_entities_list = true;
				}
				else break;
			}
			else add_to_all_entities_list = false;

			players.insert({ player->get_id(), player });
			instances.players.insert(player);

			created = true;
		}

		break;
	}
	case ENTITY_TYPE_LEVEL:
	{
		if (auto entity = utils::rtti::safe_cast<game_entity>(base))
		{
			if (auto entity_item = entity->get_item())
				entities_by_item.insert({ entity_item, entity });
			else break;

			all_entities.insert({ base->get_sync_id(), entity });
			instances.level_entities.insert(entity);

			created = add_to_all_entities_list = true;
		}

		break;
	}
	}

	if (!created)
	{
		delete base;
		return nullptr;
	}

	if (add_to_all_entities_list)
		all_entities.insert({ base->get_sync_id(), base });

	return base;
}

void game_level::clear()
{
	if (localplayer)
	{
		remove_entity(localplayer);

		localplayer = nullptr;
	}

	for (const auto& [sid, entity_base] : all_entities)
		delete entity_base;

	player_spawns.clear();
	all_entities.clear();
	entities_by_item.clear();
	players.clear();
	instances.players.clear();
	instances.level_entities.clear();
	local_streamed_entities.clear();
}

game_player* game_level::add_localplayer()
{
	return (localplayer = add_player(g_client->get_id(), g_client->get_sync_id()));
}

game_player* game_level::add_player(PLAYER_ID id, SYNC_ID sid)
{
	if (has_player(id))
		return nullptr;

	const bool is_local = g_client->has_sync_id(sid);

	return utils::rtti::safe_cast<game_player>(add_entity_base(is_local ? new game_player(id, sid) : new game_player(id, sid, default_spawn)));
}

game_entity* game_level::add_level_entity(int subtype, int16_t item_id, SYNC_ID sid)
{
	return utils::rtti::safe_cast<game_entity>(add_entity_base(new game_entity(subtype, item_id, sid)));
}

game_entity* game_level::spawn_level_entity(int16_t obj_id, SYNC_ID sid, const game_vec3d& vec)
{
	if (auto subtype = g_level->get_level_entity_subtype_from_obj_id(obj_id); subtype != ENTITY_LEVEL_TYPE_NONE)
		if (auto entity = utils::rtti::safe_cast<game_entity>(add_entity_base(new game_entity(obj_id, subtype, sid, vec))))
		{
			if (entity->get_item())
				return entity;

			delete entity;
		}

	return nullptr;
}

void game_level::remove_entity(game_entity_base* base)
{
	switch (base->get_type())
	{
	case ENTITY_TYPE_PLAYER:
	{
		if (auto player = utils::rtti::safe_cast<game_player>(base))
		{
			players.erase(player->get_id());
			instances.players.erase(player);

			prof::print(YELLOW, "[PLAYERS] We destroyed player {:#x}", player->get_id());
		}

		break;
	}
	case ENTITY_TYPE_LEVEL:
	{
		if (auto entity = utils::rtti::safe_cast<game_entity>(base))
			instances.level_entities.erase(entity);

		break;
	}
	default: return;
	}

	if (auto it = local_streamed_entities.find(base); it != local_streamed_entities.end())
		local_streamed_entities.erase(it);

	entities_by_item.erase(base->get_item());
	all_entities.erase(base->get_sync_id());

	delete base;
}

void game_level::set_level(const std::string& v, bool restart)
{
	if (!restart && !filename.compare(v))
		return;

	filename = v;
	loaded = false;
	change = true;

	g_client->set_ready_status(false);

	clear();

	if (!add_localplayer())
		prof::critical_error("Localplayer could not be created after level load alert");

	localplayer->update_localplayer_instance_info(true);
}

void game_level::add_streamed_entity(SYNC_ID sid)
{
	if (auto entity = get_entity_by_sid(sid))
	{
		/*if (entity->get_obj_id() == 288)
			prof::print(GREEN, "I own the gay raptor as {:#x}", game_level::LOCALPLAYER()->get_id());*/

		local_streamed_entities.insert(entity);
	}
}

void game_level::remove_streamed_entity(SYNC_ID sid)
{
	if (auto entity = get_entity_by_sid(sid))
	{
		/*if (entity->get_obj_id() == 288)
			prof::print(RED, "I no longer own the gay raptor as  {:#x}", game_level::LOCALPLAYER()->get_id());*/

		local_streamed_entities.erase(entity);
	}
}

void game_level::request_entity_ownership(game_entity_base* entity, bool acquire, int timeout)
{
	if (!entity)
		return;

	const bool is_already_streamed = is_entity_streamed(entity);

	if ((is_already_streamed && acquire) || (!is_already_streamed && !acquire))
		return;

	gns::sync::ownership info
	{
		.sid = entity->get_sync_id(),
		.timeout = timeout,
		.acquire = acquire
	};

	g_client->send_packet(ID_SYNC_OWNERSHIP, info);
}

bool game_level::is_entity_streamed(game_entity_base* entity)
{
	if (!entity)
		return false;

	return local_streamed_entities.contains(entity);
}

level_entity_type game_level::get_level_entity_subtype_from_obj_id(int obj_id)
{
	auto it = subtypes.find(obj_id);
	return (it != subtypes.end() ? it->second : ENTITY_LEVEL_TYPE_NONE);
}

game_entity_base* game_level::get_entity_by_sid(SYNC_ID sid)
{
	if (sid == localplayer->get_sync_id())
		return localplayer;

	auto it = all_entities.find(sid);
	return (it != all_entities.end() ? it->second : nullptr);
}

game_entity_base* game_level::get_entity_by_item(ITEM_INFO* item)
{
	auto it = entities_by_item.find(item);
	return (it != entities_by_item.end() ? it->second : nullptr);
}

game_player* game_level::get_player_by_id(PLAYER_ID id)
{
	auto it = players.find(id);
	return (it != players.end() ? it->second : nullptr);
}

game_player* game_level::get_player_by_item(ITEM_INFO* item)
{
	return utils::rtti::safe_cast<game_player>(get_entity_by_item(item));
}

game_player* game_level::get_player_by_name(const std::string& name)
{
	for (const auto& player : instances.players)
		if (player->get_name() == name)
			return player;

	return nullptr;
}

bool game_level::has_player(game_player* player) const
{
	return instances.players.contains(player);
}

bool game_level::has_player(PLAYER_ID id) const
{
	return players.contains(id);
}

bool game_level::has_player(ITEM_INFO* player_item)
{
	return !!get_player_by_item(player_item);
}
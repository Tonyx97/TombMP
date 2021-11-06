import utils;
import prof;

#include <shared/defs.h>
#include <shared/scripting/resource_system.h>

#include <scripting/events.h>

#include <mp/client.h>
#include <mp/game/level.h>
#include <mp/game/player.h>

#include <game/missile.h>
#include <game/types.h>

#include "hl_defs.h"

void entity_handlers::handle_packet(uint16_t pid)
{
	switch (pid)
	{
	case ID_ENTITY_HIT_DAMAGE:		return on_entity_hit_damage();
	case ID_ENTITY_EXPLODE:			return on_entity_explode();
	case ID_ENTITY_SPAWN:			return on_entity_spawn();
	case ID_ENTITY_ATTACH:			return on_entity_attach();
	}
}

void entity_handlers::on_entity_hit_damage()
{
	gns::entity::hit_damage info; g_client->read_packet_ex(info);

	auto entity = g_level->get_entity_by_sid(info.sid);
	if (!entity)
		return;

	auto localplayer = game_level::LOCALPLAYER();

	bool entity_damaged = false;

	if (entity != localplayer)
	{
		entity->set_health(entity->get_health() - info.damage);
		entity_damaged = entity->is_spawned();
	}
	else if (entity_damaged = lara.spawned)
		lara_item->hit_points -= info.damage;

	if (info.blood.create && entity_damaged)
		DoBloodSplat(info.blood.x, info.blood.y, info.blood.z, info.blood.speed, info.blood.angle, info.blood.room);
}

void entity_handlers::on_entity_explode()
{
	gns::entity::explode info; g_client->read_packet_ex(info);

	auto entity = g_level->get_entity_by_sid(info.sid);
	if (!entity)
		return;

	auto localplayer = game_level::LOCALPLAYER();
	auto entity_item = entity->get_item();

	if (entity != localplayer)
	{
		if (entity->is_spawned())
		{
			auto entity_item_id = entity->get_item_id();

			if (entity->get_type() == ENTITY_TYPE_PLAYER)
			{
				entity->add_entity_flags(ENTITY_FLAG_INVISIBLE);
				entity->set_health(DONT_TARGET);
				entity->set_collidable(0);

				ExplodingDeath(entity_item_id, 0xffffffff, 0);
			}
			else CreatureDie(entity_item_id, true, false, false);
		}
	}
	else if (lara.spawned)
	{
		localplayer->set_health(DONT_TARGET);
		localplayer->set_collidable(0);
		localplayer->set_entity_flags(ENTITY_FLAG_INVISIBLE);

		ExplodingDeath(localplayer->get_item_id(), 0xffffffff, 0);
	}
}

void entity_handlers::on_entity_spawn()
{
	gns::entity::spawn info; g_client->read_packet_ex(info);

	g_level->spawn_level_entity(info.obj_id, info.sid, info.vec);
}

void entity_handlers::on_entity_attach()
{
	gns::entity::attach info; g_client->read_packet_ex(info);

	if (auto a = g_level->get_entity_by_sid(info.a_sid))
		if (auto b = g_level->get_entity_by_sid(info.b_sid))
			attach_entities(a->get_item(), b->get_item(), info.local_pos, info.local_rot);
}
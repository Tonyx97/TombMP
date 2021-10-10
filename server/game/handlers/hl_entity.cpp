import utils;

#include <shared/defs.h>
#include <shared/scripting/resource_system.h>

#include <scripting/events.h>

#include <server/server.h>

#include <game/level.h>
#include <game/player.h>

#include <net/net_player.h>

#include "hl_defs.h"

void entity_handlers::handle_packet(net_player* n_player, uint16_t pid)
{
	if (!n_player)
		return;

	switch (pid)
	{
	case ID_ENTITY_HIT_DAMAGE:		return on_entity_hit_damage(n_player);
	case ID_ENTITY_EXPLODE:			return on_entity_explode(n_player);
	}
}

void entity_handlers::on_entity_hit_damage(net_player* n_player)
{
	gns::entity::hit_damage info; g_server->read_packet_ex(info);

	if (auto entity = g_level->get_entity_by_sid(info.sid))
	{
		auto old_hp = entity->get_health();

		entity->set_health(old_hp - info.damage);

		g_server->send_packet_broadcast_ex(ID_ENTITY_HIT_DAMAGE, n_player->get_sys_address(), true, info);

		if (old_hp > 0)
			g_resource->trigger_event(events::entity::ON_ENTITY_DAMAGE, g_level->get_entity_by_sid(info.attacker_sid), entity, info.damage, info.weapon);
	}
}

void entity_handlers::on_entity_explode(net_player* n_player)
{
	gns::entity::explode info; g_server->read_packet_ex(info);

	if (auto entity = g_level->get_entity_by_sid(info.sid))
	{
		entity->set_health(0);

		g_server->send_packet_broadcast_ex(ID_ENTITY_EXPLODE, n_player->get_sys_address(), true, info);
	}
}
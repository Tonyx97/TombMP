import utils;
import prof;

#include <shared/defs.h>
#include <shared/scripting/resource.h>
#include <shared/scripting/resource_system.h>
#include <shared/scripting/events.h>

#include <mp/client.h>
#include <mp/game/player.h>
#include <mp/game/level.h>

#include <ui/ui.h>

#include <specific/standard.h>
#include <specific/output.h>
#include <specific/global.h>

#include <game/camera.h>
#include <game/laraanim.h>
#include <game/control.h>
#include <game/laramisc.h>
#include <game/lara2gun.h>

#include "hl_defs.h"

void player_handlers::handle_packet(uint16_t pid)
{
	switch (pid)
	{
	case ID_PLAYER_SYNC_PLAYERS:	return on_player_sync_with_players();
	case ID_PLAYER_SPAWN:			return on_player_spawn();
	case ID_PLAYER_INFO:			return on_player_info();
	case ID_PLAYER_DEATH:			return on_player_death();
	case ID_PLAYER_RESPAWN:			return on_player_respawn();
	}
}

void player_handlers::on_player_sync_with_players()
{
	auto bs = g_client->get_current_bs();

	int size = 0; bs->Read(size);
	SYNC_ID local_sid = 0; bs->Read(local_sid);

	auto localplayer = game_level::LOCALPLAYER();

	g_client->set_sync_id(local_sid);

	localplayer->update_localplayer_instance_info();

	for (int i = 0; i < size; ++i)
	{
		gns::player::initial_sync info; bs->Read(info);

		if (auto player = g_level->add_player(info.id, info.sid))
			player->set_name(*info.name);
	}
}

void player_handlers::on_player_spawn()
{
	gns::player::spawn info; g_client->read_packet_ex(info);

	if (info.id == g_client->get_id())
	{
		if (!lara.spawned)
		{
			/*if (info.set_position)
			{
				utils::mem::move(lara_item->pos.x_pos, info.position);

				UpdateLaraRoomTeleport(lara_item, 0);
			}

			if (info.set_rotation)
				utils::mem::move(lara_item->pos.x_rot, info.rotation);*/

			lara_item->hit_points = info.health;

			lara.spawned = true;
		}
	}
	else if (auto player = g_level->get_player_by_id(info.id))
		player->set_name(*info.name);
}

void player_handlers::on_player_info()
{
	gns::player::info info; g_client->read_packet_ex(info);

	if (auto player = g_level->get_player_by_id(info.id))
	{
		if (!player->is_spawned())
			player->spawn();
		
		// item variables

		player->set_entity_flags(info.entity_flags);
		player->set_floor(info.floor);
		player->set_touch_bits(info.touch_bits);
		player->set_mesh_bits(info.mesh_bits);
		player->set_new_room(info.room);
		player->set_speed(info.speed);
		player->set_fallspeed(info.fallspeed);
		player->set_health(info.health);

		// interpolation
		
		// avoid snapping due to the game engine teleporting lara
		// when climbing etc

		if (info.position.distance(player->get_position()) > 250)
			player->set_position(info.position);

		player->set_next_position(info.position);
		player->set_rotation(info.rotation);
		player->set_anim_id(info.anim);
		player->set_anim_frame(info.anim_frame);

		// temp variables

		player->set_head_rotation(info.head_rotation);
		player->set_torso_rotation(info.torso_rotation);
		player->set_left_arm_info(&info.left_arm);
		player->set_right_arm_info(&info.right_arm);
		player->set_back_gun(info.back_gun);
		player->set_hit_direction(info.hit_direction);
		player->set_hit_frame(info.hit_frame);
		player->set_gun_status(info.gun_status);
		player->set_gun_type(info.gun_type);
		player->set_smoke_weapon(info.smoke_weapon);
		player->set_smoke_count_l(info.smoke_count_l);
		player->set_smoke_count_r(info.smoke_count_r);
		player->set_flare_age(info.flare_age);
		player->set_ducked(info.ducked);
		player->set_underwater(info.underwater);
		player->set_burning(info.burning);
		player->set_electric(info.electric);
		player->set_flare_in_hand(info.flare_in_hand);
		player->set_weapon_item_current_anim_state(info.weapon_item_current_anim_state);
		player->set_water_status(info.water_status);
		player->set_meshes_offsets(info.meshes_offsets);
		player->set_fire_color(info.fire_r, info.fire_g, info.fire_b);
		player->set_ping(info.ping);
		player->set_collidable(info.collidable);
		player->set_hair_enabled(info.hair_enabled);

		if (info.vehicle != 0)
		{
			if (auto vehicle_entity = g_level->get_entity_by_sid(info.vehicle))
				player->set_vehicle(vehicle_entity);
		}
		else player->set_vehicle(nullptr);

		// update linked lists

		player->update_linked_lists();

		if (info.respawn)
		{
			player->set_flags(0);
			player->get_item()->ai_bits &= ~EXPLODED;

			if (lara.target == player->get_item())
				lara.target = nullptr;
		}
	}
}

void player_handlers::on_player_death()
{
	PLAYER_ID id; g_client->read_packet_ex(id);

	g_resource->trigger_event(events::player::ON_PLAYER_DIED, g_level->get_player_by_id(id));
}

void player_handlers::on_player_respawn()
{
	gns::player::respawn info; g_client->read_packet_ex(info);

	game_vec3d game_vec;

	auto default_spawn = g_level->get_default_spawn();
	auto localplayer = game_level::LOCALPLAYER();

	if (info.set_position)
	{
		game_vec.pos.x = info.position.x;
		game_vec.pos.y = info.position.y;
		game_vec.pos.z = info.position.z;
		game_vec.room = info.room;
	}
	else
	{
		game_vec.pos.x = default_spawn.pos.x;
		game_vec.pos.y = default_spawn.pos.y;
		game_vec.pos.z = default_spawn.pos.z;
		game_vec.room = default_spawn.room;
	}

	game_vec.rot.x = info.rotation.x;
	game_vec.rot.y = info.rotation.y;
	game_vec.rot.z = info.rotation.z;

	ResetLaraInfo();

	lara.respawned = true;
	lara.skidoo = NO_ITEM;

	ResetLaraState();

	lara_item->hit_points = info.health;
	lara_item->pos = { game_vec.pos.x, game_vec.pos.y, game_vec.pos.z, game_vec.rot.x, game_vec.rot.y, game_vec.rot.z };

	if (lara_item->room_number != game_vec.room)
		ItemNewRoom(lara.item_number, game_vec.room);

	InitialiseResetCamera();

	lara_item->ai_bits &= ~EXPLODED;

	localplayer->set_flags(0);
	localplayer->set_collidable(1);
	localplayer->remove_entity_flags(ENTITY_FLAG_INVISIBLE);
}
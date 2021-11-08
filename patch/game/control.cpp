import utils;
import prof;

#include <specific/standard.h>
#include <specific/stypes.h>
#include <specific/input.h>
#include <specific/global.h>

#include <shared/scripting/resource.h>
#include <shared/scripting/resource_system.h>

#include <scripting/events.h>

#include <keycode/keycode.h>

#include <mp/client.h>
#include <mp/game/player.h>
#include <mp/game/level.h>

#include "objects.h"
#include "lara.h"
#include "laraanim.h"
#include "camera.h"
#include "control.h"
#include "gameflow.h"
#include "inventry.h"
#include "hair.h"
#include "laramisc.h"
#include "effect2.h"
#include "pickup.h"
#include "moveblok.h"
#include "traps.h"
#include "lot.h"
#include "health.h"
#include "fish.h"
#include "game.h"
#include "physics.h"

#define DEATH_WAIT		(10 * 30)
#define SIGN_BIT		0x80000000
#define RETURN_TARGET	{ target->x = x; target->y = y; target->z = z; target->room_number = last_room;   return 0; }
#define RETURN_TARGET2	{ target->x = x; target->y = y; target->z = z; target->room_number = room_number; return -1; }
#define TRIG_BITS(T)	((T & 0x3fff) >> 10)

long CheckNoColCeilingTriangle(FLOOR_INFO* floor, long x, long z);
long CheckNoColFloorTriangle(FLOOR_INFO* floor, long x, long z);

int los_rooms[20],
	number_los_rooms = 0;

int32_t ControlPhase(int32_t nframes)
{
	static int framecount = 0;

	if (nframes > MAX_FRAMES)
		nframes = MAX_FRAMES;

	framecount += nframes;

	for (; framecount > 0; framecount -= 2)
	{
		if (g_level->is_change_requested())
			return LEVEL_CHANGE;

		update_input(Inventory_Displaying);

		if ((input & IN_OPTION || overlay_flag <= 0) && !lara.extra_anim)
		{
			if (overlay_flag <= 0)
			{
				overlay_flag = 1;

				init_inventory(INV_GAME_MODE);
			}
			else overlay_flag = 0;
		}

		ClearDynamics();

		auto item_num = next_item_active;

		while (item_num != NO_ITEM)
		{
			auto next = items[item_num].next_active;

			if (auto control_fn = objects[items[item_num].object_number].control)
				control_fn(item_num);

			item_num = next;
		}

		item_num = next_fx_active;

		while (item_num != NO_ITEM)
		{
			auto next = effects[item_num].next_active;

			if (auto control_fn = objects[effects[item_num].object_number].control)
				control_fn(item_num);

			item_num = next;
		}

		if (KillEverythingFlag)
			KillEverything();

		if (smoke_count_l) --smoke_count_l;
		if (smoke_count_r) --smoke_count_r;
		if (splash_count)  --splash_count;

		if (lara.has_fired && ((wibble & 127) == 0))
		{
			AlertNearbyGuards(lara_item);

			lara.has_fired = 0;
		}

		if (lara.poisoned >= 16 && (wibble & 255) == 0)
		{
			if (lara.poisoned > 0x100)
				lara.poisoned = 0x100;

			lara_item->hit_points -= lara.poisoned >> 4;
		}

		for (int i = 0; i < 15; ++i)
			GotJointPos[i] = 0;

		lara_item->item_flags[0] = 0;

		if (lara.spawned)
		{
			LaraControl();
			HairControl(lara_item, (vec3d*)g_hair, (int_vec3*)g_hair_vel);
		}

		if (lara.electric)
		{
			if (lara.electric >= 12)
				TriggerDynamicLight(lara_item->pos.x_pos, lara_item->pos.y_pos, lara_item->pos.z_pos, (GetRandomControl() & 3) + 8, 0, (GetRandomControl() & 7) + 8, (GetRandomControl() & 7) + 16);
			else TriggerDynamicLight(lara_item->pos.x_pos, lara_item->pos.y_pos, lara_item->pos.z_pos, 25 - (lara.electric << 1) + (GetRandomControl() & 1), (16 - lara.electric) + (GetRandomControl() & 7), 32 - lara.electric, 31);

			g_audio->play_sound(128, { lara_item->pos.x_pos, lara_item->pos.y_pos, lara_item->pos.z_pos });
		}

		CalculateCamera();

		wibble += 4;
		wibble &= 0xfc;

		if (enable_lara_breath)
			LaraBreath(lara_item);

		UpdateSparks();
		UpdateSplashes();
		UpdateBats();
		SoundEffects();

		--health_bar_timer;
	}

	if (lara_item->hit_points <= 0 && !lara.dead)
	{
		g_client->send_packet(ID_PLAYER_DEATH);

		lara.dead = true;
	}
	else if (lara_item->hit_points > 0 && lara.dead)
		lara.dead = false;

	auto localplayer = game_level::LOCALPLAYER();

	if (lara.skidoo == NO_ITEM)
	{
		if (lara.water_status != LARA_CHEAT && (localplayer->get_entity_flags() & ENTITY_FLAG_FLY_CHEAT))
		{
			lara_item->pos.y_pos -= STEP_L * 2;

			if (lara.water_status != LARA_CHEAT)
			{
				lara.water_status = LARA_CHEAT;
				lara_item->frame_number = SWIMGLIDE_F;
				lara_item->anim_number = SWIMGLIDE_A;
				lara_item->current_anim_state = AS_SWIM;
				lara_item->goal_anim_state = AS_SWIM;
				lara_item->gravity_status = 0;
				lara_item->pos.x_rot = 30 * ONE_DEGREE;
				lara_item->fallspeed = 30;
				lara.air = LARA_AIR;
				lara.torso_x_rot = lara.torso_y_rot = 0;
				lara.head_x_rot = lara.head_y_rot = 0;
			}
		}
		else if (lara.water_status == LARA_CHEAT && !(localplayer->get_entity_flags() & ENTITY_FLAG_FLY_CHEAT))
		{
			lara_item->frame_number = STOP_F;
			lara_item->anim_number = STOP_A;
			lara_item->pos.x_rot = lara_item->pos.z_rot = 0;

			lara.water_status = LARA_ABOVEWATER;
			lara.torso_x_rot = lara.torso_y_rot = 0;
			lara.head_x_rot = lara.head_y_rot = 0;
			lara.gun_status = LG_ARMLESS;
			lara.mesh_effects = 0;

			LaraInitialiseMeshes();
		}
	}

	g_resource->trigger_event(events::engine::ON_CONTROL_PHASE);

	if (lara.dead && lara_item)
		lara_item->hit_points = 0;

	// send localplayer info to server

	gns::player::info info;

	utils::mem::move_ex(info.position, lara_item->pos.x_pos);
	utils::mem::move_ex(info.rotation, lara_item->pos.x_rot);
	utils::mem::move_ex(info.head_rotation, lara.head_y_rot);
	utils::mem::move_ex(info.torso_rotation, lara.torso_y_rot);
	utils::mem::move_ex(info.left_arm.rotation, lara.left_arm.y_rot);
	utils::mem::move_ex(info.right_arm.rotation, lara.right_arm.y_rot);

	if (lara.skidoo != NO_ITEM)
	{
		if (auto vehicle_entity = g_level->get_entity_by_item(&items[lara.skidoo]))
		{
			localplayer->set_vehicle(vehicle_entity);

			info.vehicle = vehicle_entity->get_sync_id();
		}
		else info.vehicle = 0;
	}
	else info.vehicle = 0;

	info.entity_flags = localplayer->get_entity_flags();
	info.floor = lara_item->floor;
	info.touch_bits = lara_item->touch_bits;
	info.mesh_bits = lara_item->mesh_bits;
	info.anim = lara_item->anim_number;
	info.anim_frame = lara_item->frame_number;
	info.room = lara_item->room_number;
	info.speed = lara_item->speed;
	info.fallspeed = lara_item->fallspeed;
	info.health = lara_item->hit_points;
	info.collidable = !!lara_item->collidable;
	info.back_gun = lara.back_gun;
	info.hit_direction = lara.hit_direction;
	info.hit_frame = lara.hit_frame;
	info.gun_status = lara.gun_status;
	info.gun_type = lara.gun_type;
	info.smoke_weapon = smoke_weapon;
	info.smoke_count_l = smoke_count_l;
	info.smoke_count_r = smoke_count_r;
	info.left_arm.anim = lara.left_arm.anim_number;
	info.left_arm.frame_base = (uint32_t)lara.left_arm.frame_base - (uint32_t)anims;
	info.left_arm.frame = lara.left_arm.frame_number;
	info.left_arm.flash_gun = lara.left_arm.flash_gun;
	info.right_arm.anim = lara.right_arm.anim_number;
	info.right_arm.frame_base = (uint32_t)lara.right_arm.frame_base - (uint32_t)anims;
	info.right_arm.frame = lara.right_arm.frame_number;
	info.right_arm.flash_gun = lara.right_arm.flash_gun;
	info.ducked = lara.is_ducked;
	info.underwater = lara.underwater;
	info.burning = lara.burn;
	info.fire_r = lara.burn_red;
	info.fire_g = lara.burn_green;
	info.fire_b = lara.burn_blue;
	info.electric = lara.electric;
	info.respawn = lara.respawned;
	info.flare_in_hand = lara.flare_control_left;
	info.flare_age = lara.flare_age;
	info.weapon_item_current_anim_state = lara.weapon_item != NO_ITEM ? items[lara.weapon_item].current_anim_state : -1;
	info.water_status = lara.water_status;

	lara.respawned = false;

	for (int i = 0; i < MAX_LARA_MESHES; ++i)
		info.meshes_offsets[i] = (uint32_t)lara.mesh_ptrs[i] - (uint32_t)meshes;
	
	g_client->send_packet_reliability(ID_PLAYER_INFO, UNRELIABLE_SEQUENCED, info);

	localplayer->set_ping(g_client->get_ping());
	localplayer->set_head_rotation(info.head_rotation);
	localplayer->set_torso_rotation(info.torso_rotation);
	localplayer->set_left_arm_info(&info.left_arm);
	localplayer->set_right_arm_info(&info.right_arm);
	localplayer->set_back_gun(info.back_gun);
	localplayer->set_hit_direction(info.hit_direction);
	localplayer->set_hit_frame(info.hit_frame);
	localplayer->set_gun_status(info.gun_status);
	localplayer->set_gun_type(info.gun_type);
	localplayer->set_smoke_weapon(info.smoke_weapon);
	localplayer->set_smoke_count_l(info.smoke_count_l);
	localplayer->set_smoke_count_r(info.smoke_count_r);
	localplayer->set_ducked(info.ducked);
	localplayer->set_underwater(info.underwater);
	localplayer->set_burning(info.burning);
	localplayer->set_electric(info.electric);
	localplayer->set_meshes_offsets(info.meshes_offsets);
	localplayer->set_flare_in_hand(info.flare_in_hand);
	localplayer->set_flare_age(info.flare_age);

	dispatch_entities_attachments();

	g_level->for_each_streamed_entity([&](game_entity_base* entity_base)
	{
		entity_base->sync();
	});

	return 0;
}

void AnimateItem(ITEM_INFO* item)
{
	auto anim = &anims[item->anim_number];

	item->touch_bits = item->hit_status = 0;
	++item->frame_number;

	if (anim->number_changes > 0)
	{
		if (GetChange(item, anim))
		{
			anim = &anims[item->anim_number];

			item->current_anim_state = anim->current_anim_state;

			if (item->required_anim_state == item->current_anim_state)
				item->required_anim_state = 0;
		}
	}

	if (item->frame_number > anim->frame_end)
	{
		if (anim->number_commands > 0)
		{
			auto command = &commands[anim->command_index];

			for (int i = anim->number_commands; i > 0; --i)
			{
				switch (*(command++))
				{
				case COMMAND_MOVE_ORIGIN:
					TranslateItem(item, (int32_t)*(command), (int32_t)*(command + 1), (int32_t)*(command + 2));
					command += 3;
					break;
				case COMMAND_JUMP_VELOCITY:
					item->fallspeed = *(command++);
					item->speed = *(command++);
					item->gravity_status = 1;
					break;
				case COMMAND_ATTACK_READY:
					break;
				case COMMAND_DEACTIVATE:
					item->after_death = (objects[item->object_number].intelligent ? 1 : 64);
					item->status = DEACTIVATED;
					break;
				case COMMAND_SOUND_FX:
				case COMMAND_EFFECT:
					command += 2;
					break;
				}
			}
		}

		item->anim_number = anim->jump_anim_num;
		item->frame_number = anim->jump_frame_num;

		anim = &anims[item->anim_number];

		if (item->current_anim_state != anim->current_anim_state)
		{
			item->current_anim_state = anim->current_anim_state;
			item->goal_anim_state = item->current_anim_state;
		}

		if (item->required_anim_state == item->current_anim_state)
			item->required_anim_state = 0;
	}

	if (anim->number_commands > 0)
	{
		auto command = &commands[anim->command_index];

		for (int i = anim->number_commands; i > 0; --i)
		{
			switch (*(command++))
			{
			case COMMAND_MOVE_ORIGIN:
				command += 3;
				break;
			case COMMAND_JUMP_VELOCITY:
				command += 2;
				break;
			case COMMAND_ATTACK_READY:
			case COMMAND_DEACTIVATE:
				break;
			case COMMAND_SOUND_FX:
			{
				if (item->frame_number == *(command))
				{
					int16_t type = *(command + 1) & 0xc000,
						    num = *(command + 1) & 0x3fff;

					if (objects[item->object_number].water_creature)
						g_audio->play_sound((int)num, { item->pos.x_pos, item->pos.y_pos, item->pos.z_pos });
					else if (item->room_number == NO_ROOM)
					{
						item->pos.x_pos = lara_item->pos.x_pos;
						item->pos.y_pos = lara_item->pos.y_pos - LARA_HITE;
						item->pos.z_pos = lara_item->pos.z_pos;

						g_audio->play_sound((int)num, { item->pos.x_pos, item->pos.y_pos, item->pos.z_pos });
					}
					else if (room[item->room_number].flags & UNDERWATER)
					{
						if (type == SFX_LANDANDWATER || type == SFX_WATERONLY)
							g_audio->play_sound((int)num, { item->pos.x_pos, item->pos.y_pos, item->pos.z_pos });
					}
					else if (type == SFX_LANDANDWATER || type == SFX_LANDONLY)
						g_audio->play_sound((int)num, { item->pos.x_pos, item->pos.y_pos, item->pos.z_pos });
				}

				command += 2;

				break;
			}
			case COMMAND_EFFECT:
			{
				if (item->frame_number == *(command))
				{
					FXType = command[1] & 0xc000;

					int16_t num = *(command + 1) & 0x3fff;

					(*effect_routines[(int)num])(item);
				}

				command += 2;
			}
			}
		}
	}

	if (!item->gravity_status)
	{
		int speed = anim->velocity;

		if (anim->acceleration != 0)
			speed += anim->acceleration * (item->frame_number - anim->frame_base);

		item->speed = (int16_t)(speed >> 16);
	}
	else
	{
		item->fallspeed += (item->fallspeed < FASTFALL_SPEED) ? GRAVITY : 1;
		item->pos.y_pos += item->fallspeed;
	}

	item->pos.x_pos += (phd_sin(item->pos.y_rot) * item->speed) >> W2V_SHIFT;
	item->pos.z_pos += (phd_cos(item->pos.y_rot) * item->speed) >> W2V_SHIFT;
}

int GetChange(ITEM_INFO* item, ANIM_STRUCT* anim)
{
	if (item->current_anim_state == item->goal_anim_state)
		return 0;

	auto change = (CHANGE_STRUCT*)anim->change_ptr;

	for (int i = 0; i < anim->number_changes; ++i, ++change)
	{
		if (change->goal_anim_state == item->goal_anim_state)
		{
			auto range = &ranges[change->range_index];

			for (int j = 0; j < change->number_ranges; ++j, ++range)
			{
				if (item->frame_number >= range->start_frame && item->frame_number <= range->end_frame)
				{
					item->anim_number = range->link_anim_num;
					item->frame_number = range->link_frame_num;

					return 1;
				}
			}
		}
	}

	return 0;
}

void TranslateItem(ITEM_INFO* item, int32_t x, int32_t y, int32_t z)
{
	int cosy = phd_cos(item->pos.y_rot),
		siny = phd_sin(item->pos.y_rot);

	item->pos.x_pos += (cosy * x + siny * z) >> W2V_SHIFT;
	item->pos.y_pos += y;
	item->pos.z_pos += (-siny * x + cosy * z) >> W2V_SHIFT;
}

FLOOR_INFO* FindFloor(int32_t x, int32_t y, int32_t z, int16_t* room_number)
{
	FLOOR_INFO* floor = nullptr;

	*room_number = 0;

	for (int16_t i = 0, ti = 0; i < number_rooms; ++i)
	{
		floor = GetFloor(x, y, z, &ti);

		auto temp = *room_number;

		*room_number = i;

		/*if (i != temp)
			break;*/
	}

	return floor;
}

FLOOR_INFO* GetFloor(int32_t x, int32_t y, int32_t z, int16_t* room_number)
{
	FLOOR_INFO* floor;

	auto r = &room[*room_number];

	int16_t data;

	do {
		int x_floor = (z - r->z) >> WALL_SHIFT,
			y_floor = (x - r->x) >> WALL_SHIFT;

		if (x_floor <= 0)
		{
			x_floor = 0;

			if (y_floor < 1)				  y_floor = 1;
			else if (y_floor > r->y_size - 2) y_floor = r->y_size - 2;
		}
		else if (x_floor >= r->x_size - 1)
		{
			x_floor = r->x_size - 1;

			if (y_floor < 1)				  y_floor = 1;
			else if (y_floor > r->y_size - 2) y_floor = r->y_size - 2;
		}
		else if (y_floor < 0)		   y_floor = 0;
		else if (y_floor >= r->y_size) y_floor = r->y_size - 1;

		floor = &r->floor[x_floor + (y_floor * r->x_size)];

		if ((data = GetDoor(floor)) != NO_ROOM)
		{
			*room_number = data;
			r = &room[data];
		}

	} while (data != NO_ROOM);

	if (!floor)
		return nullptr;

	if (y >= ((int32_t)floor->floor << 8))
	{
		do {
			if (floor->pit_room == NO_ROOM)
				return floor;

			int ret_val = CheckNoColFloorTriangle(floor, x, z);
			if (ret_val == 1)
				break;

			if (ret_val == -1 && y < r->minfloor)
				break;

			*room_number = floor->pit_room;

			r = &room[floor->pit_room];
			floor = &r->floor[((z - r->z) >> WALL_SHIFT) + (((x - r->x) >> WALL_SHIFT) * r->x_size)];
		} while (y >= ((int32_t)floor->floor << 8));
	}
	else if (y < ((int32_t)floor->ceiling << 8))
	{
		do {
			if (floor->sky_room == NO_ROOM)
				return floor;

			int ret_val = CheckNoColCeilingTriangle(floor, x, z);
			if (ret_val == 1)
				break;

			if (ret_val == -1 && y >= r->maxceiling)
				break;

			*room_number = floor->sky_room;

			r = &room[floor->sky_room];
			floor = &r->floor[((z - r->z) >> WALL_SHIFT) + ((x - r->x) >> WALL_SHIFT) * r->x_size];
		} while (y < ((int32_t)floor->ceiling << 8));
	}

	return floor;
}

int32_t GetWaterHeight(int32_t x, int32_t y, int32_t z, int16_t room_number)
{
	FLOOR_INFO* floor;

	auto r = &room[room_number];

	int16_t data;

	do
	{
		int x_floor = (z - r->z) >> WALL_SHIFT,
			y_floor = (x - r->x) >> WALL_SHIFT;

		if (x_floor <= 0)
		{
			x_floor = 0;

			if (y_floor < 1)				  y_floor = 1;
			else if (y_floor > r->y_size - 2) y_floor = r->y_size - 2;
		}
		else if (x_floor >= r->x_size - 1)
		{
			x_floor = r->x_size - 1;

			if (y_floor < 1)				  y_floor = 1;
			else if (y_floor > r->y_size - 2) y_floor = r->y_size - 2;
		}
		else if (y_floor < 0)		   y_floor = 0;
		else if (y_floor >= r->y_size) y_floor = r->y_size - 1;

		floor = &r->floor[x_floor + y_floor * r->x_size];

		if ((data = GetDoor(floor)) != NO_ROOM)
		{
			room_number = data;
			r = &room[data];
		}
	} while (data != NO_ROOM);

	if (!floor)
		return NO_HEIGHT;

	if (r->flags & (UNDERWATER | SWAMP))
	{
		while (floor->sky_room != NO_ROOM)
		{
			if (CheckNoColCeilingTriangle(floor, x, z) == 1)
				break;

			r = &room[floor->sky_room];

			if (!(r->flags & (UNDERWATER | SWAMP)))
				return (r->minfloor);

			floor = &r->floor[((z - r->z) >> WALL_SHIFT) + ((x - r->x) >> WALL_SHIFT) * r->x_size];
		}

		return r->maxceiling;
	}
	else
	{
		while (floor->pit_room != NO_ROOM)
		{
			if (CheckNoColFloorTriangle(floor, x, z) == 1)
				break;

			r = &room[floor->pit_room];

			if (r->flags & (UNDERWATER | SWAMP))
				return r->maxceiling;

			floor = &r->floor[((z - r->z) >> WALL_SHIFT) + ((x - r->x) >> WALL_SHIFT) * r->x_size];
		}
	}

	return NO_HEIGHT;
}

int32_t GetHeight(FLOOR_INFO* floor, int32_t x, int32_t y, int32_t z)
{
	int16_t* data, type, trigger;

	OnObject = 0;
	height_type = WALL;

	while (floor->pit_room != NO_ROOM)
	{
		if (CheckNoColFloorTriangle(floor, x, z) == 1)
			break;

		auto r = &room[floor->pit_room];

		floor = &r->floor[((z - r->z) >> WALL_SHIFT) + ((x - r->x) >> WALL_SHIFT) * r->x_size];
	}

	int32_t height = floor->floor << 8,
		   height2 = height;

	if (height == NO_HEIGHT)
		return height;

	if (GF_NoFloor && GF_NoFloor == height)
		height = 0x4000;

	trigger_index = nullptr;

	if (!floor->index)
		return height;

	tiltxoff = tiltyoff = 0;

	data = &floor_data[floor->index];

	do {
		type = *(data++);

		int xoff = 0,
			yoff = 0;

		switch (type & DATA_TYPE)
		{
		case SPLIT1:
		case SPLIT2:
		case NOCOLF1T:
		case NOCOLF1B:
		case NOCOLF2T:
		case NOCOLF2B:
		{
			auto tilts = *data;

			int16_t t0 = tilts & 15,
				   t1 = (tilts >> 4) & 15,
				   t2 = (tilts >> 8) & 15,
				   t3 = (tilts >> 12) & 15,
				   dx = x & 1023,
				   dz = z & 1023,
				   hadj;

			height_type = SPLIT_TRI;

			if ((type & DATA_TYPE) == SPLIT1 || (type & DATA_TYPE) == NOCOLF1T || (type & DATA_TYPE) == NOCOLF1B)
			{
				if (dx <= (1024 - dz))
				{
					hadj = (type >> 10) & 0x1f;

					if (hadj & 0x10)
						hadj |= 0xfff0;

					height += hadj << 8;
					xoff = t2 - t1;
					yoff = t0 - t1;
				}
				else
				{
					hadj = (type >> 5) & 0x1f;

					if (hadj & 0x10)
						hadj |= 0xfff0;

					height += hadj << 8;
					xoff = t3 - t0;
					yoff = t3 - t2;
				}
			}
			else
			{
				if (dx <= dz)
				{
					hadj = (type >> 10) & 0x1f;

					if (hadj & 0x10)
						hadj |= 0xfff0;

					height += hadj << 8;
					xoff = t2 - t1;
					yoff = t3 - t2;
				}
				else
				{
					hadj = (type >> 5) & 0x1f;

					if (hadj & 0x10)
						hadj |= 0xfff0;

					height += hadj << 8;
					xoff = t3 - t0;
					yoff = t0 - t1;

				}
			}

			tiltxoff = xoff;
			tiltyoff = yoff;

			if (chunky_flag)
			{
				long	h1, h2;

				hadj = (hadj >> 10) & 0x1f;

				if (hadj & 0x10)
					hadj |= 0xfff0;

				h1 = h2 = height2;
				h1 += hadj << 8;

				hadj = (hadj >> 5) & 0x1f;

				if (hadj & 0x10)
					hadj |= 0xfff0;

				h2 += hadj << 8;

				height = (h1 < h2 ? h1 : h2);
			}
			else
			{
				if (ABS(xoff) > 2 || ABS(yoff) > 2) height_type = DIAGONAL;
				else if (height_type != SPLIT_TRI)	height_type = SMALL_SLOPE;

				if (xoff < 0)
					height -= (z & (WALL_L - 1)) * xoff >> 2;
				else height += ((WALL_L - 1 - z) & (WALL_L - 1)) * xoff >> 2;

				if (yoff < 0)
					height -= (x & (WALL_L - 1)) * yoff >> 2;
				else height += ((WALL_L - 1 - x) & (WALL_L - 1)) * yoff >> 2;
			}

			++data;

			break;
		}
		case TILT_TYPE:
		{
			tiltxoff = xoff = (*data) >> 8;
			tiltyoff = yoff = (signed char)(*data);

			if (!chunky_flag || (ABS(xoff) <= 2 && ABS(yoff) <= 2))
			{
				height_type = (ABS(xoff) > 2 || ABS(yoff) > 2 ? BIG_SLOPE : SMALL_SLOPE);

				if (xoff < 0)
					height -= (z & (WALL_L - 1)) * xoff >> 2;
				else height += ((WALL_L - 1 - z) & (WALL_L - 1)) * xoff >> 2;

				if (yoff < 0)
					height -= (x & (WALL_L - 1)) * yoff >> 2;
				else height += ((WALL_L - 1 - x) & (WALL_L - 1)) * yoff >> 2;
			}

			++data;

			break;
		}
		case SPLIT3:
		case SPLIT4:
		case NOCOLC1T:
		case NOCOLC1B:
		case NOCOLC2T:
		case NOCOLC2B:
		case ROOF_TYPE:
		case DOOR_TYPE:
			++data;
			break;
		case LAVA_TYPE:
			trigger_index = data - 1;
			break;
		case CLIMB_TYPE:
		case MONKEY_TYPE:
		case MINEL_TYPE:
		case MINER_TYPE:
		{
			if (!trigger_index)
				trigger_index = data - 1;

			break;
		}
		case TRIGGER_TYPE:
		{
			if (!trigger_index)
				trigger_index = data - 1;

			++data;

			do {
				trigger = *(data++);

				if (TRIG_BITS(trigger) != TO_OBJECT)
				{
					if (TRIG_BITS(trigger) == TO_CAMERA)
						trigger = *(data++);

					continue;
				}

				auto item = &items[trigger & VALUE_BITS];

				if (objects[item->object_number].floor)
					(*objects[item->object_number].floor)(item, x, y, z, &height);
			} while (!(trigger & END_BIT));

			break;
		}
		default: S_ExitSystem();
		}
	} while (!(type & END_BIT));

	return height;
}

void RefreshCamera(int16_t type, int16_t* data)
{
	int target_ok = 2;

	int16_t trigger;

	do {
		trigger = *(data++);

		int16_t value = trigger & VALUE_BITS;

		switch (TRIG_BITS(trigger))
		{
		case TO_CAMERA:
		{
			++data;

			if (value == camera.last)
			{
				camera.number = value;

				if (camera.timer < 0 || camera.type == LOOK_CAMERA || camera.type == COMBAT_CAMERA)
				{
					camera.timer = -1;
					target_ok = 0;
					break;
				}

				camera.type = FIXED_CAMERA;

				target_ok = 1;
			}
			else target_ok = 0;

			break;
		}
		case TO_TARGET:
		{
			if (camera.type == LOOK_CAMERA || camera.type == COMBAT_CAMERA)
				break;

			camera.item = &items[value];

			break;
		}
		}
	} while (!(trigger & END_BIT));

	if (camera.item && (!target_ok || (target_ok == 2 && camera.item->looked_at && camera.item != camera.last_item)))
		camera.item = nullptr;

	if (camera.number == NO_CAMERA && camera.timer > 0)
		camera.timer = -1;
}

void TestTriggers(int16_t* data, int heavy)
{
	HeavyTriggered = 0;

	if (!heavy)
	{
		lara.climb_status = 0;
		lara.can_monkey_swing = 0;
		lara.mine_l = 0;
		lara.mine_r = 0;
	}

	if (!data)
		return;

	if ((*data & DATA_TYPE) == LAVA_TYPE)
	{
		if (!heavy && (lara_item->pos.y_pos == lara_item->floor || lara.water_status != LARA_ABOVEWATER))
			LavaBurn(lara_item);

		if (*data & END_BIT)
			return;

		++data;
	}

	if ((*data & DATA_TYPE) == CLIMB_TYPE)
	{
		if (!heavy)
		{
			int quad = (uint16_t)(lara_item->pos.y_rot + 0x2000) / 0x4000;

			if ((1 << (quad + 8)) & *data)
				lara.climb_status = 1;
		}

		if (*data & END_BIT)
			return;

		++data;
	}

	if ((*data & DATA_TYPE) == MONKEY_TYPE)
	{
		if (!heavy)			lara.can_monkey_swing = 1;
		if (*data & END_BIT) return;

		++data;
	}

	if ((*data & DATA_TYPE) == MINEL_TYPE)
	{
		if (!heavy)			 lara.mine_l = 1;
		if (*data & END_BIT) return;

		++data;
	}

	if ((*data & DATA_TYPE) == MINER_TYPE)
	{
		if (!heavy)			 lara.mine_r = 1;
		if (*data & END_BIT) return;

		++data;
	}

	int16_t type = (*(data++) >> 8) & 0x3f,
		    flags = *(data++),
		    timer = flags & 0xff,
		    value;

	if (camera.type != HEAVY_CAMERA)
		RefreshCamera(type, data);

	bool switch_off = false;

	if (heavy)
	{
		if (type != HEAVY)
			return;
	}
	else
	{
		switch (type)
		{
		case SWITCH:
		{
			value = *(data++) & VALUE_BITS;

			if (flags & ONESHOT)			  items[value].item_flags[0] = 1;
			if (!SwitchTrigger(value, timer)) return;

			switch_off = (items[value].current_anim_state == 1);

			break;
		}
		case PAD:
		case ANTIPAD:	// house training pad
			break;
		case KEY:
		{
			if (!KeyTrigger(*(data++) & VALUE_BITS))
				return;

			break;
		}
		case PICKUP:
		{
			if (!PickupTrigger(*(data++) & VALUE_BITS))
				return;

			break;
		}
		case COMBAT:
		{
			if (lara.gun_status != LG_READY)
				return;

			break;
		}
		case HEAVY:
		case DUMMY:
			return;
		}
	}

	ITEM_INFO* camera_item = nullptr;

	int neweffect = -1;

	int16_t trigger;

	bool flip = false,
		 flip_available = false;

	do {
		trigger = *(data++);
		value = trigger & VALUE_BITS;

		switch (TRIG_BITS(trigger))
		{
		case TO_OBJECT:
		{
			auto item = &items[value];

			if ((type == ANTIPAD || type == ANTITRIGGER) && (item->flags & ATONESHOT))				 break;
			if (type == SWITCH && (item->flags & SWONESHOT))										 break;
			if (type != SWITCH && type != ANTIPAD && type != ANTITRIGGER && (item->flags & ONESHOT)) break;

			if (type != ANTIPAD && type != ANTITRIGGER)
			{
				if ((item->object_number == DART_EMITTER || item->object_number == HOMING_DART_EMITTER) && item->active)
					break;

				if (item->object_number == TROPICAL_FISH || item->object_number == PIRAHNAS)
				{
					item->hit_points = timer & 7;

					SetupShoal(item->hit_points);

					timer = 0;
				}
				else if (item->object_number >= SECURITY_LASER_ALARM && item->object_number <= SECURITY_LASER_KILLER)
				{
					timer &= 7;

					if (timer == 0)
						timer = 1;

					item->hit_points = timer;
					timer = 0;
				}
				else if (item->object_number == CEILING_SPIKES || item->object_number == PENDULUM || item->object_number == FIREHEAD)
				{
					item->item_flags[0] = timer;

					if (item->object_number == FIREHEAD)
						item->hit_points = timer;

					timer = 0;
				}
			}

			item->timer = timer;

			if (timer != 1)
				item->timer *= 30;

			if (type == SWITCH)
			{
				item->flags ^= (flags & CODE_BITS);

				if (flags & ONESHOT)
					item->flags |= SWONESHOT;
			}
			else if (type == ANTIPAD || type == ANTITRIGGER)
			{
				if (item->object_number == TROPICAL_FISH || item->object_number == PIRAHNAS)
					lead_info[item->hit_points].on = 0;

				if (item->object_number == EARTHQUAKE)
				{
					item->item_flags[0] = 0;
					item->item_flags[1] = 100;
				}

				item->flags &= ~(CODE_BITS | REVERSE);

				if (flags & ONESHOT)
					item->flags |= ATONESHOT;
			}
			else if (flags & CODE_BITS)
				item->flags |= (flags & CODE_BITS);

			if ((item->flags & CODE_BITS) == CODE_BITS)
			{
				if (flags & ONESHOT)
					item->flags |= ONESHOT;

				if (!item->active)
				{
					if (objects[item->object_number].intelligent)
					{
						if (item->status == NOT_ACTIVE)
						{
							item->touch_bits = 0;
							item->status = ACTIVE;

							AddActiveItem(value);

							if (item->object_number != TARGETS)
								EnableBaddieAI(value, 1);
						}
						else if (item->status == INVISIBLE)
						{
							item->touch_bits = 0;
							item->status = EnableBaddieAI(value, 0) ? ACTIVE : INVISIBLE;

							AddActiveItem(value);
						}
					}
					else
					{
						item->touch_bits = 0;
						item->status = ACTIVE;

						AddActiveItem(value);

						HeavyTriggered = heavy;
					}
				}
			}

			if (auto entity = g_level->get_entity_by_item(item))
				entity->sync(true);

			break;
		}
		case TO_CAMERA:
		{
			trigger = *(data++);

			int16_t camera_flags = trigger,
				   camera_timer = trigger & 0xff;

			if (camera.fixed[value].flags & ONESHOT)
				break;

			camera.number = value;

			if (camera.type == LOOK_CAMERA || camera.type == COMBAT_CAMERA) break;
			if (type == COMBAT)												break;
			if (type == SWITCH && timer && switch_off)						break;

			if (camera.number != camera.last || type == SWITCH)
			{
				camera.timer = camera_timer * 30;

				if (camera_flags & ONESHOT)
					camera.fixed[camera.number].flags |= ONESHOT;

				camera.speed = ((camera_flags & CODE_BITS) >> 6) + 1;
				camera.type = (heavy) ? HEAVY_CAMERA : FIXED_CAMERA;
			}

			break;
		}
		case TO_TARGET:
			camera_item = &items[value];
			break;
		case TO_SINK:
			lara.current_active = value + 1;
			break;
		case TO_FLIPMAP:
		{
			flip_available = true;

			if (flipmap[value] & ONESHOT)
				break;

			if (type == SWITCH)			flipmap[value] ^= (flags & CODE_BITS);
			else if (flags & CODE_BITS) flipmap[value] |= (flags & CODE_BITS);

			if ((flipmap[value] & CODE_BITS) == CODE_BITS)
			{
				if (flags & ONESHOT) flipmap[value] |= ONESHOT;
				if (!flip_status)	 flip = true;
			}
			else if (flip_status)
				flip = true;

			break;
		}
		case TO_FLIPON:
		{
			flip_available = true;

			if ((flipmap[value] & CODE_BITS) == CODE_BITS && !flip_status)
				flip = true;

			break;
		}
		case TO_FLIPOFF:
		{
			flip_available = true;

			if ((flipmap[value] & CODE_BITS) == CODE_BITS && flip_status)
				flip = true;

			break;
		}
		case TO_FLIPEFFECT:
		{
			neweffect = value;
			break;
		}
		case TO_FINISH:
			FinishLevel();
			break;
		case TO_CD:
		case TO_SECRET:
			break;
		case TO_BODYBAG:
			ClearBodyBag();
			break;
		}
	} while (!(trigger & END_BIT));

	if (camera_item && (camera.type == FIXED_CAMERA || camera.type == HEAVY_CAMERA))
		camera.item = camera_item;

	if (flip)
		FlipMap();

	if (neweffect != -1 && (flip || !flip_available))
	{
		flipeffect = neweffect;
		fliptimer = 0;
	}
}

bool TriggerActive(ITEM_INFO* item)
{
	bool ok = (item->flags & REVERSE) ? 0 : 1;

	if ((item->flags & CODE_BITS) != CODE_BITS)
		return !ok;

	if (!item->timer)
		return ok;

	if (item->timer == -1)
		return !ok;

	if (--item->timer == 0)
		item->timer = -1;

	return ok;
}

int32_t GetCeiling(FLOOR_INFO* floor, int32_t x, int32_t y, int32_t z)
{
	CeilingObject = nullptr;

	auto f = floor;

	while (f->sky_room != NO_ROOM)
	{
		if (CheckNoColCeilingTriangle(floor, x, z) == 1)
			break;

		auto r = &room[f->sky_room];

		f = &r->floor[((z - r->z) >> WALL_SHIFT) + ((x - r->x) >> WALL_SHIFT) * r->x_size];
	}

	int32_t height = f->ceiling << 8;

	if (height == NO_HEIGHT)
		return height;

	if (f->index)
	{
		auto data = &floor_data[f->index];
		auto hadj = *(data++);

		int16_t ended = 0;

		int16_t type = hadj & DATA_TYPE;

		if (type == TILT_TYPE || type == SPLIT1 || type == SPLIT2 || type == NOCOLF1T || type == NOCOLF1B || type == NOCOLF2T || type == NOCOLF2B)	// gibby
		{
			ended = hadj & END_BIT;
			++data;
			hadj = *(data++);
			type = hadj & DATA_TYPE;
		}

		if (ended == 0 && (type == ROOF_TYPE || type == SPLIT3 || type == SPLIT4 || type == NOCOLC1T || type == NOCOLC1B || type == NOCOLC2T || type == NOCOLC2B))
		{
			int xoff = 0,
				yoff = 0;

			if (type != ROOF_TYPE)
			{
				int16_t tilts = *data,
					   t0 = -(tilts & 15),
					   t1 = -((tilts >> 4) & 15),
					   t2 = -((tilts >> 8) & 15),
					   t3 = -((tilts >> 12) & 15),
					   dx = x & 1023,
					   dz = z & 1023;

				if (type == SPLIT3 || type == NOCOLC1T || type == NOCOLC1B)
				{
					if (dx <= (1024 - dz))
					{
						hadj = (hadj >> 10) & 0x1f;

						if (hadj & 0x10)
							hadj |= 0xfff0;

						height += hadj << 8;
						xoff = t2 - t1;
						yoff = t3 - t2;
					}
					else
					{
						hadj = (hadj >> 5) & 0x1f;

						if (hadj & 0x10)
							hadj |= 0xfff0;

						height += hadj << 8;
						xoff = t3 - t0;
						yoff = t0 - t1;
					}
				}
				else
				{
					if (dx <= dz)
					{
						hadj = (hadj >> 10) & 0x1f;

						if (hadj & 0x10)
							hadj |= 0xfff0;

						height += hadj << 8;
						xoff = t2 - t1;
						yoff = t0 - t1;
					}
					else
					{
						hadj = (hadj >> 5) & 0x1f;

						if (hadj & 0x10)
							hadj |= 0xfff0;

						height += hadj << 8;
						xoff = t3 - t0;
						yoff = t3 - t2;
					}
				}

				if (chunky_flag)
				{
					hadj = (hadj >> 10) & 0x1f;

					if (hadj & 0x10)
						hadj |= 0xfff0;

					int h1 = (f->ceiling << 8) + hadj << 8,
						h2 = h1;

					hadj = (hadj >> 5) & 0x1f;

					if (hadj & 0x10)
						hadj |= 0xfff0;

					height = (h1 > h2 ? h1 : h2);
				}
			}
			else
			{
				xoff = (*data) >> 8;
				yoff = (signed char)(*data);
			}

			if (!chunky_flag)
			{
				if (xoff < 0)
					height += (z & (WALL_L - 1)) * xoff >> 2;
				else height -= ((WALL_L - 1 - z) & (WALL_L - 1)) * xoff >> 2;

				if (yoff >= 0)
					height -= (x & (WALL_L - 1)) * yoff >> 2;
				else height += ((WALL_L - 1 - x) & (WALL_L - 1)) * yoff >> 2;
			}
		}
	}

	while (floor->pit_room != NO_ROOM)
	{
		if (CheckNoColFloorTriangle(floor, x, z) == 1)
			break;

		auto r = &room[floor->pit_room];

		floor = &r->floor[((z - r->z) >> WALL_SHIFT) + ((x - r->x) >> WALL_SHIFT) * r->x_size];
	}

	if (!floor->index)
		return (height);

	auto data = &floor_data[floor->index];

	int16_t trigger, type;

	do {
		type = *(data++);

		switch (type & DATA_TYPE)
		{
		case SPLIT1:
		case SPLIT2:
		case SPLIT3:
		case SPLIT4:
		case NOCOLF1T:
		case NOCOLF1B:
		case NOCOLF2T:
		case NOCOLF2B:
		case NOCOLC1T:
		case NOCOLC1B:
		case NOCOLC2T:
		case NOCOLC2B:
		case DOOR_TYPE:
		case TILT_TYPE:
		case ROOF_TYPE:
			++data;
			break;
		case LAVA_TYPE:
		case CLIMB_TYPE:
		case MONKEY_TYPE:
		case MINEL_TYPE:
		case MINER_TYPE:
			break;
		case TRIGGER_TYPE:
		{
			++data;

			do {
				trigger = *(data++);

				if (TRIG_BITS(trigger) != TO_OBJECT)
				{
					if (TRIG_BITS(trigger) == TO_CAMERA)
						trigger = *(data++);

					continue;
				}

				auto item = &items[trigger & VALUE_BITS];

				if (auto ceiling_fn = objects[item->object_number].ceiling)
					ceiling_fn(item, x, y, z, &height);
			} while (!(trigger & END_BIT));

			break;
		}
		default: S_ExitSystem();
		}
	} while (!(type & END_BIT));

	return height;
}

int16_t GetDoor(FLOOR_INFO* floor)
{
	if (!floor->index)
		return NO_ROOM;

	if (floor->index < 0 || floor->index >= floor_data_count)
		return NO_ROOM;

	auto data = &floor_data[floor->index];
	auto type = *(data++);

	if ((type & DATA_TYPE) == TILT_TYPE ||
		(type & DATA_TYPE) == SPLIT1 ||
		(type & DATA_TYPE) == SPLIT2 ||
		(type & DATA_TYPE) == NOCOLF1B ||
		(type & DATA_TYPE) == NOCOLF1T ||
		(type & DATA_TYPE) == NOCOLF2B ||
		(type & DATA_TYPE) == NOCOLF2T)
	{
		if (type & END_BIT)
			return NO_ROOM;

		++data;

		type = *(data++);
	}

	if ((type & DATA_TYPE) == ROOF_TYPE ||
		(type & DATA_TYPE) == SPLIT3 ||
		(type & DATA_TYPE) == SPLIT4 ||
		(type & DATA_TYPE) == NOCOLC1B ||
		(type & DATA_TYPE) == NOCOLC1T ||
		(type & DATA_TYPE) == NOCOLC2B || (type & DATA_TYPE) == NOCOLC2T)
	{
		if (type & END_BIT)
			return NO_ROOM;

		++data;

		type = *(data++);
	}

	return ((type & DATA_TYPE) == DOOR_TYPE ? *data : NO_ROOM);
}

int LOS(GAME_VECTOR* start, GAME_VECTOR* target)
{
	int los1, los2;

	if ((ABS(target->z - start->z)) > (ABS(target->x - start->x)))
	{
		los1 = xLOS(start, target);
		los2 = zLOS(start, target);
	}
	else
	{
		los1 = zLOS(start, target);
		los2 = xLOS(start, target);
	}

	if (!los2)
		return 0;

	GetFloor(target->x, target->y, target->z, &target->room_number);

	return (ClipTarget(start, target) && los1 == 1 && los2 == 1);
}

int zLOS(GAME_VECTOR* start, GAME_VECTOR* target)
{
	int dz = target->z - start->z;
	if (dz == 0)
		return 1;

	int dx = ((target->x - start->x) << WALL_SHIFT) / dz,
		dy = ((target->y - start->y) << WALL_SHIFT) / dz;

	int16_t room_number = start->room_number,
		   last_room = room_number;

	los_rooms[0] = room_number;

	number_los_rooms = 1;

	if (dz < 0)
	{
		int z = start->z & ~(WALL_L - 1),
			x = start->x + (dx * (z - start->z) >> WALL_SHIFT),
			y = start->y + (dy * (z - start->z) >> WALL_SHIFT);

		while (z > target->z)
		{
			auto floor = GetFloor(x, y, z, &room_number);

			if (y > GetHeight(floor, x, y, z) || y < GetCeiling(floor, x, y, z))
				RETURN_TARGET2;

			if (room_number != last_room)
			{
				los_rooms[number_los_rooms++] = room_number;
				last_room = room_number;
			}

			floor = GetFloor(x, y, z - 1, &room_number);

			if (y > GetHeight(floor, x, y, z - 1) || y < GetCeiling(floor, x, y, z - 1))
				RETURN_TARGET;

			z -= WALL_L;
			x -= dx;
			y -= dy;
		}
	}
	else
	{
		int z = start->z | (WALL_L - 1),
			x = start->x + (dx * (z - start->z) >> WALL_SHIFT),
			y = start->y + (dy * (z - start->z) >> WALL_SHIFT);

		while (z < target->z)
		{
			auto floor = GetFloor(x, y, z, &room_number);

			if (y > GetHeight(floor, x, y, z) || y < GetCeiling(floor, x, y, z))
				RETURN_TARGET2;

			if (room_number != last_room)
			{
				los_rooms[number_los_rooms++] = room_number;
				last_room = room_number;
			}

			floor = GetFloor(x, y, z + 1, &room_number);
			if (y > GetHeight(floor, x, y, z + 1) || y < GetCeiling(floor, x, y, z + 1))
				RETURN_TARGET;

			z += WALL_L;
			x += dx;
			y += dy;
		}
	}

	target->room_number = room_number;

	return 1;
}

int xLOS(GAME_VECTOR* start, GAME_VECTOR* target)
{
	int dx = target->x - start->x;
	if (dx == 0)
		return 1;

	int dy = ((target->y - start->y) << WALL_SHIFT) / dx,
		dz = ((target->z - start->z) << WALL_SHIFT) / dx;

	int16_t room_number = start->room_number,
		   last_room = room_number;

	los_rooms[0] = room_number;

	number_los_rooms = 1;

	if (dx < 0)
	{
		int x = start->x & ~(WALL_L - 1),
			y = start->y + (dy * (x - start->x) >> WALL_SHIFT),
			z = start->z + (dz * (x - start->x) >> WALL_SHIFT);

		while (x > target->x)
		{
			auto floor = GetFloor(x, y, z, &room_number);

			if (y > GetHeight(floor, x, y, z) || y < GetCeiling(floor, x, y, z))
				RETURN_TARGET2;

			if (room_number != last_room)
			{
				los_rooms[number_los_rooms++] = room_number;
				last_room = room_number;
			}

			floor = GetFloor(x - 1, y, z, &room_number);

			if (y > GetHeight(floor, x - 1, y, z) || y < GetCeiling(floor, x - 1, y, z))
				RETURN_TARGET;

			x -= WALL_L;
			y -= dy;
			z -= dz;
		}
	}
	else
	{
		int x = start->x | (WALL_L - 1),
			y = start->y + (dy * (x - start->x) >> WALL_SHIFT),
			z = start->z + (dz * (x - start->x) >> WALL_SHIFT);

		while (x < target->x)
		{
			auto floor = GetFloor(x, y, z, &room_number);

			if (y > GetHeight(floor, x, y, z) || y < GetCeiling(floor, x, y, z))
				RETURN_TARGET2;

			if (room_number != last_room)
			{
				los_rooms[number_los_rooms++] = room_number;
				last_room = room_number;
			}

			floor = GetFloor(x + 1, y, z, &room_number);

			if (y > GetHeight(floor, x + 1, y, z) || y < GetCeiling(floor, x + 1, y, z))
				RETURN_TARGET;

			x += WALL_L;
			y += dy;
			z += dz;
		}
	}

	target->room_number = room_number;

	return 1;
}

int ClipTarget(GAME_VECTOR* start, GAME_VECTOR* target)
{
	auto room_no = target->room_number;
	auto floor = GetFloor(target->x, target->y, target->z, &room_no);

	int height = GetHeight(floor, target->x, target->y, target->z);

	if (target->y > height)
	{
		GAME_VECTOR src
		{
			start->x + (((target->x - start->x) * 7) >> 3),
			start->y + (((target->y - start->y) * 7) >> 3),
			start->z + (((target->z - start->z) * 7) >> 3)
		};

		int dx, dy, dz;

		for (int i = 3; i > 0; --i)
		{
			dx = src.x + (((target->x - src.x) * i) >> 2);
			dy = src.y + (((target->y - src.y) * i) >> 2);
			dz = src.z + (((target->z - src.z) * i) >> 2);
			floor = GetFloor(dx, dy, dz, &room_no);
			height = GetHeight(floor, dx, dy, dz);

			if (dy < height)
			{
				target->x = dx;
				target->y = dy;
				target->z = dz;
				target->room_number = room_no;

				return 0;
			}
		}

		target->x = dx;
		target->y = dy;
		target->z = dz;
		target->room_number = room_no;

		return 0;
	}

	room_no = target->room_number;
	floor = GetFloor(target->x, target->y, target->z, &room_no);
	height = GetCeiling(floor, target->x, target->y, target->z);

	if (target->y < height)
	{
		GAME_VECTOR src
		{
			start->x + (((target->x - start->x) * 7) >> 3),
			start->y + (((target->y - start->y) * 7) >> 3),
			start->z + (((target->z - start->z) * 7) >> 3)
		};

		int dx, dy, dz;

		for (int i = 3; i > 0; --i)
		{
			dx = src.x + (((target->x - src.x) * i) >> 2);
			dy = src.y + (((target->y - src.y) * i) >> 2);
			dz = src.z + (((target->z - src.z) * i) >> 2);
			floor = GetFloor(dx, dy, dz, &room_no);
			height = GetCeiling(floor, dx, dy, dz);

			if (dy > height)
			{
				target->x = dx;
				target->y = dy;
				target->z = dz;
				target->room_number = room_no;

				return 0;
			}
		}

		target->x = dx;
		target->y = dy;
		target->z = dz;
		target->room_number = room_no;

		return 0;
	}

	return 1;
}

int ObjectOnLOS(GAME_VECTOR* start, GAME_VECTOR* target)
{
	int16_t* bounds,
		   * zextent,
		   * xextent;

	int dx = target->x - start->x,
		dy = target->y - start->y,
		dz = target->z - start->z;

	for (int i = 0; i < number_los_rooms; ++i)
	{
		for (auto item_number = room[los_rooms[i]].item_number; item_number != NO_ITEM; item_number = items[item_number].next_item)
		{
			auto item = &items[item_number];

			if (item->status == DEACTIVATED)
				continue;

			if (item->object_number != SMASH_WINDOW &&
				item->object_number != SMASH_OBJECT1 &&
				item->object_number != SMASH_OBJECT2 &&
				item->object_number != SMASH_OBJECT3 &&
				item->object_number != CARCASS &&
				item->object_number != EXTRAFX6)
				continue;

			bounds = GetBoundsAccurate(item);

			if (((uint16_t)(item->pos.y_rot + 0x2000) / 0x4000) & 1)
			{
				zextent = &bounds[0];
				xextent = &bounds[4];
			}
			else
			{
				zextent = &bounds[4];
				xextent = &bounds[0];
			}

			int failure = 0;

			if (ABS(dz) > ABS(dx))
			{
				int distance = item->pos.z_pos + zextent[0] - start->z;

				for (int j = 0; j < 2; ++j)
				{
					if ((distance & SIGN_BIT) == (dz & SIGN_BIT))
					{
						int y = dy * distance / dz;

						if (y > item->pos.y_pos + bounds[2] - start->y && y < item->pos.y_pos + bounds[3] - start->y)
						{
							int x = dx * distance / dz;

							if (x < item->pos.x_pos + xextent[0] - start->x)	  failure |= 0x1;
							else if (x > item->pos.x_pos + xextent[1] - start->x) failure |= 0x2;
							else												  return item_number;
						}
					}

					distance = item->pos.z_pos + zextent[1] - start->z;
				}

				if (failure == 0x3)
					return item_number;
			}
			else
			{
				int distance = item->pos.x_pos + xextent[0] - start->x;

				for (int j = 0; j < 2; ++j)
				{
					if ((distance & SIGN_BIT) == (dx & SIGN_BIT))
					{
						int y = dy * distance / dx;

						if (y > item->pos.y_pos + bounds[2] - start->y && y < item->pos.y_pos + bounds[3] - start->y)
						{
							int z = dz * distance / dx;

							if (z < item->pos.z_pos + zextent[0] - start->z)	  failure |= 0x1;
							else if (z > item->pos.z_pos + zextent[1] - start->z) failure |= 0x2;
							else												  return item_number;
						}
					}

					distance = item->pos.x_pos + xextent[1] - start->x;
				}

				if (failure == 0x3)
					return item_number;
			}
		}
	}

	return NO_ITEM;
}

void FlipMap(bool sync)
{
	auto r = room;

	if (sync && g_client->get_game_settings().flip_map_sync)
		g_client->send_packet(ID_LEVEL_FLIP_MAP);

	for (int i = 0; i < number_rooms; ++i, ++r)
	{
		if (r->flipped_room < 0)
			continue;

		RemoveRoomFlipItems(r);

		auto flipped = &room[r->flipped_room];

		ROOM_INFO temp;

		memcpy(&temp, r, sizeof(ROOM_INFO));
		memcpy(r, flipped, sizeof(ROOM_INFO));
		memcpy(flipped, &temp, sizeof(ROOM_INFO));

		r->flipped_room = flipped->flipped_room;
		r->item_number = flipped->item_number;
		r->fx_number = flipped->fx_number;

		flipped->flipped_room = -1;

		AddRoomFlipItems(r);
	}

	flip_status = !flip_status;
}

void RemoveRoomFlipItems(ROOM_INFO* r)
{
	for (auto item_num = r->item_number; item_num != NO_ITEM; item_num = items[item_num].next_item)
		if (auto item = &items[item_num]; objects[item->object_number].control == MovableBlock)
			AlterFloorHeight(item, 1024);
		else if ((item->flags & ONESHOT) && objects[item->object_number].intelligent && item->hit_points <= 0)
		{
			RemoveDrawnItem(item_num);

			item->flags |= KILLED_ITEM;
		}
}

void AddRoomFlipItems(ROOM_INFO* r)
{
	for (auto item_num = r->item_number; item_num != NO_ITEM; item_num = items[item_num].next_item)
		if (auto item = &items[item_num]; objects[item->object_number].control == MovableBlock)
			AlterFloorHeight(item, -1024);
}

long CheckNoColFloorTriangle(FLOOR_INFO* floor, long x, long z)
{
	if (!floor->index)
		return 0;

	auto data = &floor_data[floor->index];
	auto type = *(data) & DATA_TYPE;

	if (type == NOCOLF1T || type == NOCOLF1B || type == NOCOLF2T || type == NOCOLF2B)
	{
		int dx = x & 1023,
			dz = z & 1023;

		if ((type == NOCOLF1T && dx <= (1024 - dz)) ||
			(type == NOCOLF1B && dx > (1024 - dz)) ||
			(type == NOCOLF2T && dx <= dz) ||
			(type == NOCOLF2B && dx > dz))
			return -1;
			
		return 1;
	}

	return 0;
}

long CheckNoColCeilingTriangle(FLOOR_INFO* floor, long x, long z)
{
	if (!floor->index)
		return 0;

	auto data = &floor_data[floor->index];
	auto type = *(data) & DATA_TYPE;

	if (type == TILT_TYPE || type == SPLIT1 || type == SPLIT2 || type == NOCOLF1T || type == NOCOLF1B || type == NOCOLF2T || type == NOCOLF2B)
	{
		if (*(data) & END_BIT)
			return 0;

		type = *(data + 2) & DATA_TYPE;
	}

	if (type == NOCOLC1T || type == NOCOLC1B || type == NOCOLC2T || type == NOCOLC2B)
	{
		int dx = x & 1023,
			dz = z & 1023;

		if ((type == NOCOLC1T && dx <= (1024 - dz)) ||
			(type == NOCOLC1B && dx > (1024 - dz)) ||
			(type == NOCOLC2T && dx <= dz) ||
			(type == NOCOLC2B && dx > dz))
			return -1;

		else return 1;
	}

	return 0;
}

int32_t IsRoomOutside(int32_t x, int32_t y, int32_t z)
{
	auto offset = OutsideRoomOffsets[((x >> 12) * 27) + (z >> 12)];

	if (offset == -1)
		return -2;

	if (offset < 0)
	{
		auto r = &room[(offset & 0x7fff)];

		if (y > r->maxceiling &&
			y < r->minfloor &&
			(z > (r->z + 1024) && z < r->z + ((r->x_size - 1) * 1024)) &&
			(x > (r->x + 1024) && x < r->x + ((r->y_size - 1) * 1024)))
		{
			int16_t room_number = offset & 0x7fff;

			auto floor = GetFloor(x, y, z, &room_number);

			int height = GetHeight(floor, x, y, z);
			if (height == NO_HEIGHT || y > height)
				return -2;

			if (y < GetCeiling(floor, x, y, z))
				return -2;

			if (!(r->flags & (NOT_INSIDE | UNDERWATER)))
				return -3;

			IsRoomOutsideNo = offset & 0x7fff;

			return 1;
		}
		else return -2;
	}
	else
	{
		unsigned char* s = &OutsideRoomTable[offset];

		while (*s != 0xff)
		{
			auto r = &room[*s];

			if (y > r->maxceiling &&
				y < r->minfloor &&
				(z > (r->z + 1024) && z < r->z + ((r->x_size - 1) * 1024)) &&
				(x > (r->x + 1024) && x < r->x + ((r->y_size - 1) * 1024)))
			{
				int16_t room_number = *s;

				auto floor = GetFloor(x, y, z, &room_number);
				auto height = GetHeight(floor, x, y, z);

				if (height == NO_HEIGHT || y > height || y < GetCeiling(floor, x, y, z))
					return -2;

				if (!(r->flags & (NOT_INSIDE | UNDERWATER)))
					return -3;

				IsRoomOutsideNo = *s;

				return 1;
			}

			++s;
		}

		return -2;
	}
}
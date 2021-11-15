import utils;

#include <specific/standard.h>
#include <specific/global.h>
#include <specific/input.h>

#include <shared/scripting/resource.h>
#include <shared/scripting/resource_system.h>

#include <mp/client.h>
#include <mp/game/player.h>
#include <mp/game/level.h>

#include "objects.h"
#include "laraanim.h"
#include "camera.h"
#include "control.h"
#include "invfunc.h"
#include "inventry.h"
#include "larafunc.h"
#include "laramisc.h"
#include "laraswim.h"
#include "larasurf.h"
#include "lara2gun.h"
#include "health.h"
#include "physics.h"
#include "game.h"
#include "types.h"

#define WADE_DEPTH (STEP_L)
#define SWIM_DEPTH 730

COLL_INFO mycoll,
		* coll = &mycoll;

void LaraInitialiseMeshes()
{
	for (int i = 0; i < MAX_LARA_MESHES; ++i)
		lara.mesh_ptrs[i] = objects[LARA].mesh_ptr[i] = objects[407].mesh_ptr[i];
}

void LaraCheatGetStuff()
{
	Inv_AddItem(M16_ITEM);
	Inv_AddItem(SHOTGUN_ITEM);
	Inv_AddItem(UZI_ITEM);
	Inv_AddItem(MAGNUM_ITEM);
	Inv_AddItem(GUN_ITEM);
	Inv_AddItem(FLAREBOX_ITEM);
	Inv_AddItem(MEDI_ITEM);
	Inv_AddItem(BIGMEDI_ITEM);
	Inv_AddItem(KEY_ITEM1);
	Inv_AddItem(KEY_ITEM2);
	Inv_AddItem(KEY_ITEM3);
	Inv_AddItem(KEY_ITEM4);
	Inv_AddItem(PUZZLE_ITEM1);
	Inv_AddItem(PUZZLE_ITEM2);
	Inv_AddItem(PUZZLE_ITEM3);
	Inv_AddItem(PUZZLE_ITEM4);
	Inv_AddItem(PICKUP_ITEM1);
	Inv_AddItem(PICKUP_ITEM2);

	lara.magnums.ammo = MAGNUM_AMMO_CLIP * 50;
	lara.uzis.ammo = UZI_AMMO_CLIP * 50;
	lara.shotgun.ammo = SHOTGUN_AMMO_CLIP * 50;
	lara.m16.ammo = SHOTGUN_AMMO_CLIP * 50;
}

void LaraCheat(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraUnderWater(item, coll);
}

void LaraCheatyBits()
{
#ifdef _DEBUG
	if (input & IN_D)
	{
		LaraCheatGetStuff();

		lara_item->hit_points = LARA_HITPOINTS;
	}
#endif
}

void LaraControl()
{
	LaraCheatyBits();

	auto item = lara_item;

	lara.last_pos.x = item->pos.x_pos;
	lara.last_pos.y = item->pos.y_pos;
	lara.last_pos.z = item->pos.z_pos;

	if (lara.gun_status == LG_HANDSBUSY &&
		lara_item->current_anim_state == AS_STOP &&
		lara_item->goal_anim_state == AS_STOP &&
		lara_item->anim_number == BREATH_A &&
		lara.extra_anim == 0 &&
		lara_item->gravity_status == 0)
	{
		lara.gun_status = LG_ARMLESS;
	}

	if (item->current_anim_state != AS_DASH && lara.dash < LARA_DASH_TIME)
		++lara.dash;

	lara.is_ducked = 0;

	int room_water_state = room[item->room_number].flags & (UNDERWATER | SWAMP),
		wd = GetWaterDepth(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, item->room_number),
		wh = GetWaterHeight(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, item->room_number),
		hfw = (wh != NO_HEIGHT ? item->pos.y_pos - wh : NO_HEIGHT);

	lara.water_surface_dist = -hfw;

	if (lara.skidoo == NO_ITEM)
		WadeSplash(item, wh, wd);

	if (lara.skidoo == NO_ITEM && !lara.extra_anim)
	{
		switch (lara.water_status)
		{
		case LARA_ABOVEWATER:
		{
			if (hfw == NO_HEIGHT || hfw < WADE_DEPTH || lara.skidoo != NO_ITEM)
				break;

			if (wd > SWIM_DEPTH - STEP_L && !(room_water_state & SWAMP))
			{
				if (room_water_state)
				{
					lara.air = LARA_AIR;
					lara.water_status = LARA_UNDERWATER;

					item->gravity_status = 0;
					item->pos.y_pos += 100;

					UpdateLaraRoom(lara_item, 0);

					g_audio->stop_sound(30);

					if (item->current_anim_state == AS_SWANDIVE)
					{
						item->pos.x_rot = -45 * ONE_DEGREE;
						item->goal_anim_state = AS_DIVE;

						AnimateLara(item);

						item->fallspeed = item->fallspeed * 2;
					}
					else if (item->current_anim_state == AS_FASTDIVE)
					{
						item->pos.x_rot = -85 * ONE_DEGREE;
						item->goal_anim_state = AS_DIVE;

						AnimateLara(item);

						item->fallspeed = item->fallspeed * 2;
					}
					else
					{
						item->pos.x_rot = -45 * ONE_DEGREE;
						item->frame_number = JUMPIN_F;
						item->anim_number = JUMPIN_A;
						item->current_anim_state = AS_DIVE;
						item->goal_anim_state = AS_SWIM;
						item->fallspeed = (item->fallspeed * 3) / 2;
					}

					lara.torso_x_rot = lara.torso_y_rot = 0;
					lara.head_x_rot = lara.head_y_rot = 0;

					Splash(lara_item);
				}
			}
			else if (hfw > WADE_DEPTH)
			{
				lara.water_status = LARA_WADE;

				if (!item->gravity_status)
					item->goal_anim_state = AS_STOP;
				else if (room_water_state & SWAMP)
				{
					if (item->current_anim_state == AS_SWANDIVE || item->current_anim_state == AS_FASTDIVE)
						item->pos.y_pos = wh + 1000;

					item->goal_anim_state = AS_WADE;
					item->current_anim_state = AS_WADE;
					item->anim_number = WADE_A;
					item->frame_number = WADE_F;
				}
			}

			break;
		}
		case LARA_WADE:
		{
			camera.target_elevation = -22 * ONE_DEGREE;

			if (hfw < WADE_DEPTH)
			{
				lara.water_status = LARA_ABOVEWATER;

				if (item->current_anim_state == AS_WADE)
					item->goal_anim_state = AS_RUN;
			}
			else if (hfw > SWIM_DEPTH && !(room_water_state & SWAMP))
			{
				lara.water_status = LARA_SURFACE;

				item->pos.y_pos = item->pos.y_pos + 1 - hfw;

				if (item->current_anim_state == AS_BACK)
				{
					item->frame_number = SURFTRD2BK_F;
					item->anim_number = SURFTRD2BK_A;
					item->goal_anim_state = AS_SURFBACK;
					item->current_anim_state = AS_SURFBACK;
				}
				else if (item->current_anim_state == AS_STEPRIGHT)
				{
					item->frame_number = SURFRIGHT_F;
					item->anim_number = SURFRIGHT_A;
					item->goal_anim_state = AS_SURFRIGHT;
					item->current_anim_state = AS_SURFRIGHT;
				}
				else if (item->current_anim_state == AS_STEPLEFT)
				{
					item->frame_number = SURFLEFT_F;
					item->anim_number = SURFLEFT_A;
					item->goal_anim_state = AS_SURFLEFT;
					item->current_anim_state = AS_SURFLEFT;
				}
				else
				{
					item->frame_number = SURFSWIM_F;
					item->anim_number = SURFSWIM_A;
					item->goal_anim_state = AS_SURFSWIM;
					item->current_anim_state = AS_SURFSWIM;
				}

				item->gravity_status = 0;
				item->fallspeed = 0;
				item->pos.x_rot = lara_item->pos.z_rot = 0;

				lara.dive_count = 0;
				lara.torso_x_rot = lara.torso_y_rot = 0;
				lara.head_x_rot = lara.head_y_rot = 0;

				UpdateLaraRoom(item, -LARA_HITE / 2);
			}

			break;
		}
		case LARA_SURFACE:
		{
			if (!room_water_state)
			{
				if (hfw > WADE_DEPTH)
				{
					lara.water_status = LARA_WADE;

					item->anim_number = BREATH_A;
					item->frame_number = BREATH_F;
					item->current_anim_state = AS_STOP;

					if (item->current_anim_state == AS_SURFBACK)		item->goal_anim_state = AS_BACK;
					else if (item->current_anim_state == AS_SURFLEFT)	item->goal_anim_state = AS_STEPLEFT;
					else if (item->current_anim_state == AS_SURFRIGHT)  item->goal_anim_state = AS_STEPRIGHT;
					else												item->goal_anim_state = AS_WADE;

					AnimateItem(item);
				}
				else
				{
					lara.water_status = LARA_ABOVEWATER;

					item->frame_number = FALLDOWN_F;
					item->anim_number = FALLDOWN_A;
					item->goal_anim_state = AS_FORWARDJUMP;
					item->current_anim_state = AS_FORWARDJUMP;
					item->speed = item->fallspeed / 4;
					item->gravity_status = 1;
				}

				item->fallspeed = 0;
				item->pos.x_rot = lara_item->pos.z_rot = 0;

				lara.torso_x_rot = lara.torso_y_rot = 0;
				lara.head_x_rot = lara.head_y_rot = 0;
			}

			break;
		}
		case LARA_UNDERWATER:
		{
			if (!room_water_state)
			{
				if (wd != NO_HEIGHT && ABS(hfw) < STEP_L)
				{
					item->pos.y_pos = item->pos.y_pos + 1 - hfw;
					item->frame_number = SURFTREAD_F;
					item->anim_number = SURFTREAD_A;
					item->goal_anim_state = AS_SURFTREAD;
					item->current_anim_state = AS_SURFTREAD;
					item->fallspeed = 0;
					item->pos.x_rot = lara_item->pos.z_rot = 0;

					lara.water_status = LARA_SURFACE;
					lara.dive_count = DIVE_COUNT + 1;
					lara.torso_x_rot = lara.torso_y_rot = 0;
					lara.head_x_rot = lara.head_y_rot = 0;

					UpdateLaraRoom(item, -LARA_HITE / 2);
					g_audio->play_sound(36, { lara_item->pos.x_pos, lara_item->pos.y_pos, lara_item->pos.z_pos });
				}
				else
				{
					item->frame_number = FALLDOWN_F;
					item->anim_number = FALLDOWN_A;
					item->goal_anim_state = AS_FORWARDJUMP;
					item->current_anim_state = AS_FORWARDJUMP;
					item->speed = item->fallspeed / 4;
					item->gravity_status = 1;
					item->fallspeed = 0;
					item->pos.x_rot = lara_item->pos.z_rot = 0;

					lara.water_status = LARA_ABOVEWATER;
					lara.torso_x_rot = lara.torso_y_rot = 0;
					lara.head_x_rot = lara.head_y_rot = 0;
				}
			}
		}
		}
	}

	if (item->hit_points <= 0)
	{
		item->hit_points = -1;

		if ((lara_item->flags & ONESHOT) || (game_level::LOCALPLAYER()->get_entity_flags() & ENTITY_FLAG_INVISIBLE))
			return;
	}
	else if (GF_NoFloor && item->pos.y_pos >= GF_NoFloor)
		item->hit_points = -1;

	switch (lara.water_status)
	{
	case LARA_WADE:
	case LARA_ABOVEWATER:
	{
		if ((room[item->room_number].flags & SWAMP) && lara.water_surface_dist < -775)
		{
			if (item->hit_points >= 0)
			{
				lara.air -= 6;

				if (lara.air < 0)
				{
					lara.air = -1;

					item->hit_points -= 10;
				}
			}
		}
		else if (lara.skidoo != NO_ITEM && items[lara.skidoo].object_number != UPV)
			lara.air = LARA_AIR;

		LaraAboveWater(item, coll);

		if (enable_cold_exposure)
		{
			if (lara.water_status == LARA_WADE)
				lara.exposure -= LARA_WADE_EXPOSURE;
			else if ((lara.exposure += LARA_HEAT_EXPOSURE) > LARA_EXPOSURE_TIME)
				lara.exposure = LARA_EXPOSURE_TIME;
		}

		break;
	}
	case LARA_UNDERWATER:
	{
		if (item->hit_points >= 0)
		{
			if (--lara.air < 0)
			{
				lara.air = -1;

				item->hit_points -= 5;
			}
		}

		LaraUnderWater(item, coll);

		if (enable_cold_exposure)
			lara.exposure -= LARA_SWIM_EXPOSURE;

		break;
	}
	case LARA_SURFACE:
	{
		if (item->hit_points >= 0)
		{
			lara.air += 10;

			if (lara.air > LARA_AIR)
				lara.air = LARA_AIR;
		}

		LaraSurface(item, coll);

		if (enable_cold_exposure)
			lara.exposure -= LARA_WADE_EXPOSURE;

		break;
	}
	case LARA_CHEAT: LaraCheat(item, coll);
	}

	if (lara.exposure < 0)
	{
		lara.exposure = -1;

		item->hit_points -= 10;
	}
}

void LaraSwapMeshExtra()
{
	if (objects[LARA_EXTRA].loaded)
		for (int i = 0; i < 15; ++i)
			lara.mesh_ptrs[i] = *(objects[LARA_EXTRA].mesh_ptr + i);
}

void AnimateLara(ITEM_INFO* item)
{
	++item->frame_number;

	auto anim = &anims[item->anim_number];

	if (anim->number_changes > 0)
	{
		if (GetChange(item, anim))
		{
			anim = &anims[item->anim_number];
			item->current_anim_state = anim->current_anim_state;
		}
	}

	if (item->frame_number > anim->frame_end)
	{
		if (anim->number_commands > 0)
		{
			auto command = anim->command_ptr;

			for (int i = anim->number_commands; i > 0; --i)
			{
				switch (*(command++))
				{
				case COMMAND_MOVE_ORIGIN:
					TranslateItem(item, (int32_t)*(command), (int32_t)*(command + 1), (int32_t)*(command + 2));
					command += 3;
					break;
				case COMMAND_JUMP_VELOCITY:
				{
					item->fallspeed = *(command++);
					item->speed = *(command++);
					item->gravity_status = 1;

					if (lara.calc_fallspeed)
					{
						item->fallspeed = lara.calc_fallspeed;
						lara.calc_fallspeed = 0;
					}

					break;
				}
				case COMMAND_ATTACK_READY:
				{
					if (lara.gun_status != LG_SPECIAL)
						lara.gun_status = LG_ARMLESS;
					break;
				}
				case COMMAND_DEACTIVATE:
					break;
				case COMMAND_SOUND_FX:
				case COMMAND_EFFECT:
					command += 2;
				}
			}
		}

		item->anim_number = anim->jump_anim_num;
		item->frame_number = anim->jump_frame_num;

		anim = &anims[item->anim_number];

		item->current_anim_state = anim->current_anim_state;
	}

	if (anim->number_commands > 0)
	{
		auto command = anim->command_ptr;

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
					int16_t type = *(command + 1) & 0xc000;

					if (type == SFX_LANDANDWATER ||
						(type == SFX_LANDONLY && (lara.water_surface_dist >= 0 || lara.water_surface_dist == NO_HEIGHT)) ||
						(type == SFX_WATERONLY && lara.water_surface_dist < 0 && lara.water_surface_dist != NO_HEIGHT && !(room[lara_item->room_number].flags & SWAMP)))
					{
						int16_t num = *(command + 1) & 0x3fff;

						g_audio->play_sound((int)num, { item->pos.x_pos, item->pos.y_pos, item->pos.z_pos });
					}
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
		if (lara.water_status == LARA_WADE && (room[item->room_number].flags & SWAMP))
		{
			int speed = anim->velocity >> 1;

			if (anim->acceleration)
				speed += (anim->acceleration * (item->frame_number - anim->frame_base)) >> 2;

			item->speed = (int16_t)(speed >> 16);
		}
		else
		{
			int speed = anim->velocity;

			if (anim->acceleration)
				speed += anim->acceleration * (item->frame_number - anim->frame_base);

			item->speed = (int16_t)(speed >> 16);
		}
	}
	else
	{
		if (room[item->room_number].flags & SWAMP)
		{
			item->speed -= item->speed >> 3;

			if (abs(item->speed) < 8)
			{
				item->speed = 0;
				item->gravity_status = 0;
			}

			if (item->fallspeed > 128)
				item->fallspeed >>= 1;

			item->fallspeed -= item->fallspeed >> 2;

			if (item->fallspeed < 4)
				item->fallspeed = 4;

			item->pos.y_pos += item->fallspeed;
		}
		else
		{
			int speed = anim->velocity + anim->acceleration * (item->frame_number - anim->frame_base - 1);

			item->speed -= (int16_t)(speed >> 16);

			speed += anim->acceleration;

			item->speed += (int16_t)(speed >> 16);
			item->fallspeed += (item->fallspeed < FASTFALL_SPEED) ? GRAVITY : 1;
			item->pos.y_pos += item->fallspeed;
		}
	}

	item->pos.x_pos += (phd_sin(lara.move_angle) * item->speed) >> W2V_SHIFT;
	item->pos.z_pos += (phd_cos(lara.move_angle) * item->speed) >> W2V_SHIFT;
}

void UseItem(int16_t object_num)
{
	if (object_num < 0 || object_num >= NUMBER_OBJECTS)
		return;

	switch (object_num)
	{
	case GUN_ITEM:
	case GUN_OPTION:	  lara.request_gun_type = LG_PISTOLS; break;
	case MAGNUM_ITEM:
	case MAGNUM_OPTION:   lara.request_gun_type = LG_MAGNUMS; break;
	case UZI_ITEM:
	case UZI_OPTION:	  lara.request_gun_type = LG_UZIS; break;
	case SHOTGUN_ITEM:
	case SHOTGUN_OPTION:  lara.request_gun_type = LG_SHOTGUN; break;
	case HARPOON_ITEM:
	case HARPOON_OPTION:  lara.request_gun_type = LG_HARPOON; break;
	case M16_ITEM:
	case M16_OPTION:	  lara.request_gun_type = LG_M16; break;
	case ROCKET_GUN_ITEM:
	case ROCKET_OPTION:   lara.request_gun_type = LG_ROCKET; break;
	case GRENADE_GUN_ITEM:
	case GRENADE_OPTION:  lara.request_gun_type = LG_GRENADE; break;
	case FLAREBOX_ITEM:
	case FLAREBOX_OPTION: lara.request_gun_type = LG_FLARE; break;
	case MEDI_ITEM:
	case MEDI_OPTION:
	{
		if ((lara_item->hit_points > 0 && lara_item->hit_points < LARA_HITPOINTS) || lara.poisoned)
		{
			lara.poisoned = 0;

			lara_item->hit_points += LARA_HITPOINTS / 2;

			if (lara_item->hit_points > LARA_HITPOINTS)
				lara_item->hit_points = LARA_HITPOINTS;

			Inv_RemoveItem(MEDI_ITEM);
			g_audio->play_sound(116);
		}

		break;
	}
	case BIGMEDI_ITEM:
	case BIGMEDI_OPTION:
	{
		if ((lara_item->hit_points > 0 && lara_item->hit_points < LARA_HITPOINTS) || lara.poisoned)
		{
			lara.poisoned = 0;

			lara_item->hit_points += LARA_HITPOINTS;

			if (lara_item->hit_points > LARA_HITPOINTS)
				lara_item->hit_points = LARA_HITPOINTS;

			Inv_RemoveItem(BIGMEDI_ITEM);

			g_audio->play_sound(116);
		}
	}
	}
}

void ControlLaraExtra(int16_t item_num)
{
	AnimateItem(&items[item_num]);
}

void InitialiseLaraLoad(int16_t item_num)
{
	auto localplayer = game_level::LOCALPLAYER();

	lara.item_number = item_num;
	lara_item = &items[item_num];

	game_vec3d spawn_location;

	utils::mem::move_ex(spawn_location.pos, lara_item->pos.x_pos);
	utils::mem::move_ex(spawn_location.rot, lara_item->pos.x_rot);

	spawn_location.room = lara_item->room_number;
	spawn_location.box = lara_item->box_number;

	g_level->set_default_spawn(spawn_location);
	g_level->add_player_spawn(spawn_location);

	localplayer->update_localplayer_instance_info();
}

void InitialiseLara()
{
	auto item = lara_item;

	item->data = (void*)&lara;
	item->collidable = 1;
	item->hit_points = LARA_HITPOINTS;
	item->carried_item = NO_ITEM;

	lara.calc_fallspeed = 0;
	lara.climb_status = 0;
	lara.pose_count = 0;
	lara.hit_frame = 0;
	lara.hit_direction = -1;
	lara.air = LARA_AIR;
	lara.dive_count = 0;
	lara.current_active = 0;
	lara.spaz_effect_count = 0;
	lara.flare_age = 0;
	lara.skidoo = NO_ITEM;
	lara.weapon_item = NO_ITEM;
	lara.back_gun = 0;
	lara.flare_frame = 0;
	lara.flare_control_left = lara.flare_control_right = 0;
	lara.extra_anim = 0;
	lara.look = 1;
	lara.burn = 0;
	lara.burn_red = 255;
	lara.burn_green = 80;
	lara.burn_blue = 48;
	lara.water_surface_dist = 100;
	lara.last_pos.x = item->pos.x_pos;
	lara.last_pos.y = item->pos.y_pos;
	lara.last_pos.z = item->pos.z_pos;
	lara.spaz_effect = nullptr;
	lara.mesh_effects = 0;
	lara.target = nullptr;
	lara.turn_rate = 0;
	lara.move_angle = 0;
	lara.head_x_rot = lara.head_y_rot = lara.head_z_rot = 0;
	lara.torso_x_rot = lara.torso_y_rot = lara.torso_z_rot = 0;
	lara.left_arm.flash_gun = lara.right_arm.flash_gun = 0;
	lara.left_arm.lock = lara.right_arm.lock = 0;
	lara.poisoned = 0;
	lara.creature = nullptr;
	lara.electric = 0;
	lara.spawned = false;
	lara.frozen = false;

	if (room[item->room_number].flags & UNDERWATER)
	{
		lara.water_status = LARA_UNDERWATER;

		item->fallspeed = 0;
		item->goal_anim_state = AS_TREAD;
		item->current_anim_state = AS_TREAD;
		item->anim_number = TREAD_A;
		item->frame_number = TREAD_F;
	}
	else
	{
		lara.water_status = LARA_ABOVEWATER;

		item->goal_anim_state = AS_STOP;
		item->current_anim_state = AS_STOP;
		item->anim_number = STOP_A;
		item->frame_number = STOP_F;
	}

	InitialiseLaraInventory();
	GF_ModifyInventory();

	camera.type = LOOK_CAMERA;

	lara.dash = LARA_DASH_TIME;
	lara.exposure = LARA_EXPOSURE_TIME;
}

void ResetLaraInfo()
{
	auto item = lara_item;

	item->carried_item = NO_ITEM;
	item->mesh_bits = 0xffffffff;
	item->touch_bits = 0;
	item->after_death = 0;
	item->il.init = 0;
	item->fired_weapon = 0;
	item->gravity_status = 0;
	item->data = nullptr;
	item->flags = 0;

	lara.mesh_ptrs[HEAD] = objects[LARA].mesh_ptr[HEAD];
	lara.mesh_ptrs[THIGH_R] = objects[LG_UNARMED].mesh_ptr[THIGH_R];
	lara.mesh_ptrs[THIGH_L] = objects[LG_UNARMED].mesh_ptr[THIGH_L];
	lara.mesh_ptrs[HAND_R] = objects[LARA].mesh_ptr[HAND_R];
	lara.mesh_ptrs[HAND_L] = objects[LARA].mesh_ptr[HAND_L];

	lara.calc_fallspeed = 0;
	lara.climb_status = 0;
	lara.pose_count = 0;
	lara.hit_frame = 0;
	lara.hit_direction = -1;
	lara.air = LARA_AIR;
	lara.dive_count = 0;
	lara.current_active = 0;
	lara.spaz_effect_count = 0;
	lara.flare_age = 0;
	lara.skidoo = NO_ITEM;
	lara.weapon_item = NO_ITEM;
	lara.back_gun = 0;
	lara.flare_frame = 0;
	lara.flare_control_left = lara.flare_control_right = 0;
	lara.extra_anim = 0;
	lara.look = 1;
	lara.burn = 0;
	lara.gun_type = 0;
	lara.last_gun_type = 0;
	lara.request_gun_type = 0;
	lara.gun_status	= 0;
	lara.burn_green = 0;
	lara.water_surface_dist = 100;
	lara.last_pos.x = item->pos.x_pos;
	lara.last_pos.y = item->pos.y_pos;
	lara.last_pos.z = item->pos.z_pos;
	lara.spaz_effect = nullptr;
	lara.mesh_effects = 0;
	lara.target = nullptr;
	lara.turn_rate = 0;
	lara.move_angle = 0;
	lara.head_x_rot = lara.head_y_rot = lara.head_z_rot = 0;
	lara.torso_x_rot = lara.torso_y_rot = lara.torso_z_rot = 0;
	lara.left_arm.flash_gun = lara.right_arm.flash_gun = 0;
	lara.left_arm.lock = lara.right_arm.lock = 0;
	lara.poisoned = 0;
	lara.creature = nullptr;
	lara.electric = 0;
	lara.respawned = false;

	if (room[item->room_number].flags & UNDERWATER)
	{
		lara.water_status = LARA_UNDERWATER;

		item->fallspeed = 0;
		item->goal_anim_state = AS_TREAD;
		item->current_anim_state = AS_TREAD;
		item->anim_number = TREAD_A;
		item->frame_number = TREAD_F;
	}
	else
	{
		lara.water_status = LARA_ABOVEWATER;

		item->goal_anim_state = AS_STOP;
		item->current_anim_state = AS_STOP;
		item->anim_number = STOP_A;
		item->frame_number = STOP_F;
	}

	camera.type = LOOK_CAMERA;
}

void InitialiseLaraInventory()
{
	Inv_RemoveAllItems();

#ifdef _DEBUG
	/*Inv_AddItem(M16_ITEM);
	Inv_AddItem(HARPOON_ITEM);
	Inv_AddItem(ROCKET_GUN_ITEM);
	Inv_AddItem(SHOTGUN_ITEM);
	Inv_AddItem(UZI_ITEM);
	Inv_AddItem(MAGNUM_ITEM);
	Inv_AddItem(GRENADE_GUN_ITEM);

	lara.magnums.ammo = MAGNUM_AMMO_CLIP * 50;
	lara.uzis.ammo = UZI_AMMO_CLIP * 50;
	lara.shotgun.ammo = SHOTGUN_AMMO_CLIP * 50;
	lara.harpoon.ammo = SHOTGUN_AMMO_CLIP * 50;
	lara.m16.ammo = SHOTGUN_AMMO_CLIP * 50;
	lara.rocket.ammo = SHOTGUN_AMMO_CLIP * 50;

	for (int i = 0; i < 10; ++i)
	{
		Inv_AddItem(FLAREBOX_ITEM);
		Inv_AddItem(MEDI_ITEM);
		Inv_AddItem(BIGMEDI_ITEM);
	}

	Inv_AddItem(GUN_ITEM);

	lara.pistols.ammo = 1000;
	lara.gun_status = LG_ARMLESS;*/
#endif

	lara.request_gun_type = lara.gun_type = lara.last_gun_type = lara.back_gun = LG_UNARMED;

	LaraInitialiseMeshes();
	InitialiseNewWeapon();
}

void (*lara_control_routines[])(ITEM_INFO*, COLL_INFO*) =
{
	lara_as_walk,
	lara_as_run,
	lara_as_stop,
	lara_as_forwardjump,
	lara_as_pose,
	lara_as_fastback,
	lara_as_turn_r,
	lara_as_turn_l,
	lara_as_death,
	lara_as_fastfall,
	lara_as_hang,
	lara_as_reach,
	lara_as_splat,
	lara_as_tread,
	lara_as_land,
	lara_as_compress,
	lara_as_back,
	lara_as_swim,
	lara_as_glide,
	lara_as_null,
	lara_as_fastturn,
	lara_as_stepright,
	lara_as_stepleft,
	lara_as_roll2,
	lara_as_slide,
	lara_as_backjump,
	lara_as_rightjump,
	lara_as_leftjump,
	lara_as_upjump,
	lara_as_fallback,
	lara_as_hangleft,
	lara_as_hangright,
	lara_as_slideback,
	lara_as_surftread,
	lara_as_surfswim,
	lara_as_dive,
	lara_as_pushblock,
	lara_as_pullblock,
	lara_as_ppready,
	lara_as_pickup,
	lara_as_switchon,
	lara_as_switchoff,
	lara_as_usekey,
	lara_as_usepuzzle,
	lara_as_uwdeath,
	lara_as_roll,
	lara_as_special,
	lara_as_surfback,
	lara_as_surfleft,
	lara_as_surfright,
	lara_as_usemidas,
	lara_as_diemidas,
	lara_as_swandive,
	lara_as_fastdive,
	lara_as_gymnast,
	lara_as_waterout,
	lara_as_climbstnc,
	lara_as_climbing,
	lara_as_climbleft,
	lara_as_climbend,
	lara_as_climbright,
	lara_as_climbdown,
	lara_as_laratest1,
	lara_as_laratest2,
	lara_as_laratest3,
	lara_as_wade,
	lara_as_waterroll,
	lara_as_pickupflare,
	lara_as_twist,
	lara_as_kick,
	lara_as_deathslide,
	lara_as_duck,
	lara_as_duck,			// roll
	lara_as_dash,
	lara_as_dashdive,
	lara_as_hang2,
	lara_as_monkeyswing,
	lara_as_monkeyl,
	lara_as_monkeyr,
	lara_as_monkey180,
	lara_as_all4s,
	lara_as_crawl,
	lara_as_hangturnl,
	lara_as_hangturnr,
	lara_as_all4turnl,
	lara_as_all4turnr,
	lara_as_crawlb,
	lara_void_func,			// AS_HANG2DUCK
	lara_void_func,			// AS_CRAWL2HANG
	lara_as_extcornerl,
	lara_as_intcornerl,
	lara_as_extcornerr,
	lara_as_intcornerr,
	lara_as_controlled,
	lara_void_func,			// AS_POLESTAT
	lara_as_poleleft,		// AS_POLELEFT
	lara_as_poleright,		// AS_POLERIGHT
	lara_void_func,			// AS_POLEUP
	lara_void_func,			// AS_POLEDOWN
};

void (*extra_control_routines[])(ITEM_INFO*, COLL_INFO*) =
{
	extra_as_breath,
	extra_as_plunger,
	extra_as_yetikill,
	extra_as_sharkkill,
	extra_as_airlock,
	extra_as_gongbong,
	extra_as_dinokill,
	extra_as_pulldagger,
	extra_as_startanim,
	extra_as_starthouse,
	extra_as_finalanim,
	extra_as_trainkill,
	extra_as_rapidsdrown
};

void (*lara_collision_routines[])(ITEM_INFO*, COLL_INFO*) =
{
	lara_col_walk,
	lara_col_run,
	lara_col_stop,
	lara_col_forwardjump,
	lara_col_pose,
	lara_col_fastback,
	lara_col_turn_r,
	lara_col_turn_l,
	lara_col_death,
	lara_col_fastfall,
	lara_col_hang,
	lara_col_reach,
	lara_col_splat,
	lara_col_tread,
	lara_col_land,
	lara_col_compress,
	lara_col_back,
	lara_col_swim,
	lara_col_glide,
	lara_col_null,
	lara_col_fastturn,
	lara_col_stepright,
	lara_col_stepleft,
	lara_col_roll2,
	lara_col_slide,
	lara_col_backjump,
	lara_col_rightjump,
	lara_col_leftjump,
	lara_col_upjump,
	lara_col_fallback,
	lara_col_hangleft,
	lara_col_hangright,
	lara_col_slideback,
	lara_col_surftread,
	lara_col_surfswim,
	lara_col_dive,
	lara_col_pushblock,
	lara_col_pullblock,
	lara_col_ppready,
	lara_col_pickup,
	lara_col_switchon,
	lara_col_switchoff,
	lara_col_usekey,
	lara_col_usepuzzle,
	lara_col_uwdeath,
	lara_col_roll,
	lara_col_special,
	lara_col_surfback,
	lara_col_surfleft,
	lara_col_surfright,
	lara_col_usemidas,
	lara_col_diemidas,
	lara_col_swandive,
	lara_col_fastdive,
	lara_col_gymnast,
	lara_col_waterout,
	lara_col_climbstnc,
	lara_col_climbing,
	lara_col_climbleft,
	lara_col_climbend,
	lara_col_climbright,
	lara_col_climbdown,
	lara_col_laratest1,
	lara_col_laratest2,
	lara_col_laratest3,
	lara_col_wade,
	lara_col_waterroll,
	lara_col_pickup,
	lara_col_twist,
	lara_col_kick,
	lara_col_deathslide,
	lara_col_duck,
	lara_col_duck,			// roll
	lara_col_dash,
	lara_col_dashdive,
	lara_col_hang2,
	lara_col_monkeyswing,
	lara_col_monkeyl,
	lara_col_monkeyr,
	lara_col_monkey180,
	lara_col_all4s,
	lara_col_crawl,
	lara_col_hangturnlr,
	lara_col_hangturnlr,
	lara_col_all4turnl,
	lara_col_all4turnr,
	lara_col_crawlb,
	lara_void_func,
	lara_col_crawl2hang,
	lara_default_col,		// ext corner l
	lara_default_col,		// int corner l
	lara_default_col,		// ext corner r
	lara_default_col,		// int corner r
	lara_default_col,		// as controlled
	lara_col_polestat,		// polestat
	lara_void_func,			// poleleft
	lara_void_func,			// poleright
	lara_col_poleup,		// poleup
	lara_col_poledown,		// poledown
};
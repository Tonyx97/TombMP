import prof;

#include <specific/standard.h>
#include <specific/input.h>
#include <specific/fn_stubs.h>

#include <mp/client.h>
#include <mp/game/level.h>
#include <mp/game/player.h>

#include "objects.h"
#include "lara.h"
#include "laraanim.h"
#include "camera.h"
#include "control.h"
#include "invfunc.h"
#include "inventry.h"
#include "sphere.h"
#include "traps.h"
#include "lot.h"
#include "laraflar.h"
#include "lara1gun.h"
#include "lara2gun.h"
#include "triboss.h"
#include "effect2.h"
#include "game.h"

#define NEAR_ANGLE (ONE_DEGREE * 15)

static constexpr int16_t HoldStates[] =
{
	AS_WALK,
	AS_STOP,
	AS_POSE,
	AS_TURN_R,
	AS_TURN_L,
	AS_BACK,
	AS_FASTTURN,
	AS_STEPLEFT,
	AS_STEPRIGHT,
	AS_WADE,
	AS_PICKUP,
	AS_SWITCHON,
	AS_SWITCHOFF,
	AS_DUCK,
	-1
};

int CheckForHoldingState(int state)
{
	auto holds = HoldStates;

	if (lara.extra_anim)
		return 0;

	while (*holds >= 0)
	{
		if (state == *holds)
			return 1;

		holds++;
	}

	return 0;
}

void LaraGun()
{
	if (lara.left_arm.flash_gun > 0)  --lara.left_arm.flash_gun;
	if (lara.right_arm.flash_gun > 0) --lara.right_arm.flash_gun;
	if (lara_item->hit_points <= 0)	  lara.gun_status = LG_ARMLESS;
	else if (lara.gun_status == LG_ARMLESS)
	{
		if (input & IN_DRAW)
			lara.request_gun_type = lara.last_gun_type;
		else if (input & IN_FLARE)
		{
			if (lara.gun_type == LG_FLARE)			 lara.gun_status = LG_UNDRAW;
			else if (Inv_RequestItem(FLAREBOX_ITEM)) lara.request_gun_type = LG_FLARE;
		}
		
		if (lara.request_gun_type != lara.gun_type || (input & IN_DRAW))
		{
			if (lara_item->current_anim_state == AS_DUCK &&
				(lara.request_gun_type == LG_SHOTGUN ||
				 lara.request_gun_type == LG_M16	||
				 lara.request_gun_type == LG_ROCKET ||
				 lara.request_gun_type == LG_GRENADE ||
				 lara.request_gun_type == LG_HARPOON))
				return;

			if (lara.request_gun_type == LG_FLARE ||
				(lara.skidoo == NO_ITEM &&
				 (lara.request_gun_type == LG_HARPOON ||
				  lara.water_status == LARA_ABOVEWATER) ||
				 (lara.water_status == LARA_WADE && lara.water_surface_dist > -weapons[lara.gun_type].gun_height)))
			{
				if (lara.gun_type == LG_FLARE)
				{
					create_flare(0);
					undraw_flare_meshes();

					lara.flare_control_left = 0;
				}

				lara.gun_type = lara.request_gun_type;

				InitialiseNewWeapon();

				lara.gun_status = LG_DRAW;
				lara.left_arm.frame_number = lara.right_arm.frame_number = 0;
			}
			else
			{
				lara.last_gun_type = lara.request_gun_type;

				if (lara.gun_type == LG_FLARE) lara.request_gun_type = LG_FLARE;
				else						   lara.gun_type = lara.request_gun_type;
			}
		}
	}
	else if (lara.gun_status == LG_READY)
	{
		if ((input & IN_FLARE) && Inv_RequestItem(FLAREBOX_ITEM))
			lara.request_gun_type = LG_FLARE;

		if ((input & IN_DRAW) || lara.request_gun_type != lara.gun_type)
			lara.gun_status = LG_UNDRAW;
		else if (lara.gun_type != LG_HARPOON &&
				 lara.water_status != LARA_ABOVEWATER &&
				 (lara.water_status != LARA_WADE || lara.water_surface_dist < -weapons[lara.gun_type].gun_height))
			lara.gun_status = LG_UNDRAW;
	}

	switch (lara.gun_status)
	{
	case LG_DRAW:
	{
		if (lara.gun_type != LG_FLARE && lara.gun_type != LG_UNARMED)
			lara.last_gun_type = lara.gun_type;

		switch (lara.gun_type)
		{
		case LG_PISTOLS:
		case LG_MAGNUMS:
		case LG_UZIS:
		{
			if (camera.type != CINEMATIC_CAMERA && camera.type != LOOK_CAMERA)
				camera.type = COMBAT_CAMERA;

			draw_pistols(lara.gun_type);

			break;
		}
		case LG_SHOTGUN:
		case LG_HARPOON:
		case LG_ROCKET:
		case LG_GRENADE:
		case LG_M16:
		{
			if (camera.type != CINEMATIC_CAMERA && camera.type != LOOK_CAMERA)
				camera.type = COMBAT_CAMERA;

			draw_shotgun(lara.gun_type);

			break;
		}
		case LG_FLARE:
			draw_flare();
			break;
		default: lara.gun_status = LG_ARMLESS;
		}

		break;
	}
	case LG_UNDRAW:
	{
		lara.mesh_ptrs[HEAD] = objects[LARA].mesh_ptr[HEAD];

		switch (lara.gun_type)
		{
		case LG_PISTOLS:
		case LG_MAGNUMS:
		case LG_UZIS:
			undraw_pistols(lara.gun_type);
			break;
		case LG_SHOTGUN:
		case LG_HARPOON:
		case LG_ROCKET:
		case LG_GRENADE:
		case LG_M16:
			undraw_shotgun(lara.gun_type);
			break;
		case LG_FLARE:
			undraw_flare();
		}

		break;
	}
	case LG_SPECIAL:
		draw_flare();
		break;
	case LG_READY:
	{
		if (lara.angry_face)
			lara.mesh_ptrs[HEAD] = (lara.pistols.ammo && (input & IN_ACTION) ? objects[UZI].mesh_ptr[HEAD] : objects[LARA].mesh_ptr[HEAD]);

		if (camera.type != CINEMATIC_CAMERA && camera.type != LOOK_CAMERA)
			camera.type = COMBAT_CAMERA;

		if (input & IN_ACTION)
		{
			AMMO_INFO* ammo;

			switch (lara.gun_type)
			{
			case LG_SHOTGUN: ammo = &lara.shotgun;	break;
			case LG_MAGNUMS: ammo = &lara.magnums;	break;
			case LG_UZIS:	 ammo = &lara.uzis;		break;
			case LG_ROCKET:  ammo = &lara.rocket;	break;
			case LG_GRENADE: ammo = &lara.grenade;	break;
			case LG_HARPOON: ammo = &lara.harpoon;	break;
			case LG_M16:	 ammo = &lara.m16;		break;
			default:		 ammo = &lara.pistols;
			}

			if (ammo->ammo <= 0)
			{
				ammo->ammo = 0;

				g_audio->play_sound(48, { lara_item->pos.x_pos, lara_item->pos.y_pos, lara_item->pos.z_pos });

				lara.request_gun_type = (Inv_RequestItem(GUN_ITEM) ? LG_PISTOLS : LG_UNARMED);

				break;
			}
		}

		switch (lara.gun_type)
		{
		case LG_PISTOLS:
		case LG_UZIS:
			PistolHandler(lara.gun_type);
			break;
		case LG_SHOTGUN:
		case LG_HARPOON:
		case LG_M16:
		case LG_ROCKET:
		case LG_GRENADE:
		case LG_MAGNUMS:
			RifleHandler(lara.gun_type);
			break;
		}

		break;
	}
	case LG_ARMLESS:
	{
		if (lara.gun_type == LG_FLARE)
		{
			if (lara.skidoo != NO_ITEM || CheckForHoldingState(lara_item->current_anim_state))
			{
				if (!lara.flare_control_left)
				{
					lara.left_arm.frame_number = FL_2HOLD_F;
					lara.flare_control_left = 1;
				}
				else if (lara.left_arm.frame_number != FL_HOLD_F && ++lara.left_arm.frame_number == FL_END_F)
					lara.left_arm.frame_number = FL_HOLD_F;
			}
			else lara.flare_control_left = 0;

			do_flare_in_hand(lara.flare_age);
			set_flare_arm(lara.left_arm.frame_number);
		}

		break;
	}
	case LG_HANDSBUSY:
	{
		if (lara.gun_type == LG_FLARE)
		{
			lara.flare_control_left = (lara.skidoo != NO_ITEM || CheckForHoldingState(lara_item->current_anim_state));

			do_flare_in_hand(lara.flare_age);
			set_flare_arm(lara.left_arm.frame_number);
		}
	}
	}
}

void InitialiseNewWeapon()
{
	lara.left_arm.frame_number = lara.right_arm.frame_number = 0;
	lara.left_arm.x_rot = lara.left_arm.y_rot = lara.left_arm.z_rot = 0;
	lara.right_arm.x_rot = lara.right_arm.y_rot = lara.right_arm.z_rot = 0;
	lara.target = nullptr;
	lara.left_arm.lock = lara.right_arm.lock = 0;
	lara.left_arm.flash_gun = lara.right_arm.flash_gun = 0;

	switch (lara.gun_type)
	{
	case LG_PISTOLS:
	case LG_UZIS:
	{
		lara.left_arm.frame_base = lara.right_arm.frame_base = objects[PISTOLS].frame_base;

		if (lara.gun_status != LG_ARMLESS)
			draw_pistol_meshes(lara.gun_type);

		break;
	}
	case LG_SHOTGUN:
	case LG_HARPOON:
	case LG_M16:
	case LG_ROCKET:
	case LG_GRENADE:
	case LG_MAGNUMS:
	{
		lara.left_arm.frame_base = lara.right_arm.frame_base = objects[WeaponObject(lara.gun_type)].frame_base;

		if (lara.gun_status != LG_ARMLESS)
			draw_shotgun_meshes(lara.gun_type);

		break;
	}
	case LG_FLARE:
	{
		lara.left_arm.frame_base = lara.right_arm.frame_base = objects[FLARE].frame_base;

		if (lara.gun_status != LG_ARMLESS)
			draw_flare_meshes();

		break;
	}
	default: lara.left_arm.frame_base = lara.right_arm.frame_base = anims[lara_item->anim_number].frame_ptr;
	}
}

void LaraTargetInfo(WEAPON_INFO* winfo)
{
	auto item = lara.target;
	if (!item)
	{
		lara.left_arm.lock = lara.right_arm.lock = 0;
		lara.target_angles[0] = lara.target_angles[1] = 0;
		return;
	}

	GAME_VECTOR src { lara_item->pos.x_pos, lara_item->pos.y_pos - 650, lara_item->pos.z_pos, lara_item->room_number },
				target;

	// this commented code is for bone targetting
	/*if (lara.target_mesh != -1)
	{
		PHD_VECTOR target_pos { 0, 0, 0 };

		GetJointAbsPosition(item, &target_pos, lara.target_mesh);

		target.x = target_pos.x;
		target.y = target_pos.y;
		target.z = target_pos.z;
		target.room_number = item->room_number;
	}
	else */find_target_point(item, &target);

	auto angles = phd_GetVectorAngles({ target.x - src.x, target.y - src.y, target.z - src.z });

	angles.x -= lara_item->pos.y_rot;
	angles.y -= lara_item->pos.x_rot;

	if (LOS(&src, &target))
	{
		if (angles.x >= winfo->lock_angles[0] &&
			angles.x <= winfo->lock_angles[1] &&
			angles.y >= winfo->lock_angles[2] &&
			angles.y <= winfo->lock_angles[3])
		{
			lara.left_arm.lock = lara.right_arm.lock = 1;
		}
		else
		{
			if (lara.left_arm.lock)
			{
				if (angles.x < winfo->left_angles[0] ||
					angles.x > winfo->left_angles[1] ||
					angles.y < winfo->left_angles[2] ||
					angles.y > winfo->left_angles[3])
				{
					lara.left_arm.lock = 0;
				}
			}

			if (lara.right_arm.lock)
			{
				if (angles.x < winfo->right_angles[0] ||
					angles.x > winfo->right_angles[1] ||
					angles.y < winfo->right_angles[2] ||
					angles.y > winfo->right_angles[3])
				{
					lara.right_arm.lock = 0;
				}
			}
		}
	}
	else lara.left_arm.lock = lara.right_arm.lock = 0;

	lara.target_angles[0] = angles.x;
	lara.target_angles[1] = angles.y;
}

void LaraGetNewTarget(WEAPON_INFO* winfo)
{
	if (lara.target)
	{
		if (auto player = g_level->get_player_by_item(lara.target); player && (player->get_entity_flags() & ENTITY_FLAG_INVISIBLE))
			lara.target = nullptr;

		return;
	}

	ITEM_INFO* item = nullptr,
			 * bestitem = nullptr;

	GAME_VECTOR	src { lara_item->pos.x_pos, lara_item->pos.y_pos - 650, lara_item->pos.z_pos, lara_item->room_number };

	int bestmesh = -1,
		bestdist = 0x7fffffff,
		maxdist = winfo->target_dist,
		maxdist2 = maxdist * maxdist;

	PHD_ANGLE bestyrot = 32767;

	const bool friendly_fire = g_client->get_game_settings().friendly_fire;

	for (auto item_num = next_item_active; item_num != NO_ITEM; item_num = item->next_active)
	{
		if (item_num == NO_ITEM || item_num == lara.item_number)
			continue;

		item = &items[item_num];

		if (objects[item->object_number].intelligent == 0)
			continue;

		if (auto player = g_level->get_player_by_item(item))
			if (!friendly_fire || (player->get_entity_flags() & ENTITY_FLAG_INVISIBLE))
				continue;

		if (lara.target == item && item->hit_points <= 0)
		{
			bestitem = nullptr;
			break;
		}

		if (item->hit_points <= 0)
			continue;

		int x = item->pos.x_pos - src.x,
			y = item->pos.y_pos - src.y,
			z = item->pos.z_pos - src.z;

		if (ABS(x) > maxdist || ABS(y) > maxdist || ABS(z) > maxdist)
			continue;

		if (int dist = x * x + y * y + z * z; dist < maxdist2)
		{
			GAME_VECTOR target;

			find_target_point(item, &target);

			// targetting bones code
			// 
			//for (int j = 0; j < 10; ++j)
			{
				/*GAME_VECTOR target;
				PHD_VECTOR target_pos;

				target_pos.x = target_pos.y = target_pos.z = 0;

				GetJointAbsPosition(item, &target_pos, j);

				target.x = target_pos.x;
				target.y = target_pos.y;
				target.z = target_pos.z;
				target.room_number = item->room_number;

				dist = target.x * target.x + target.y * target.y + target.z * target.z;*/

				if (LOS(&src, &target))
				{
					auto angles = phd_GetVectorAngles({ target.x - src.x, target.y - src.y, target.z - src.z });

					angles.x -= lara_item->pos.y_rot + lara.torso_y_rot;
					angles.y -= lara_item->pos.x_rot + lara.torso_x_rot;

					if (angles.x >= winfo->lock_angles[0] &&
						angles.x <= winfo->lock_angles[1] &&
						angles.y >= winfo->lock_angles[2] &&
						angles.y <= winfo->lock_angles[3])
					{
						if (int16_t yrot = ABS(angles.x); yrot < bestyrot + NEAR_ANGLE && dist < bestdist)
						{
							bestdist = dist;
							bestyrot = yrot;
							bestitem = item;
							//bestmesh = j;
						}
					}
				}
			}
		}
	}

	lara.target = bestitem;
	lara.target_mesh = bestmesh;

	LaraTargetInfo(winfo);
}

void find_target_point(ITEM_INFO* item, GAME_VECTOR* target)
{
	auto bounds = GetBestFrame(item);

	int x = (int32_t)((*(bounds + 0) + *(bounds + 1)) / 2),
		y = (int32_t)(*(bounds + 2) + (*(bounds + 3) - *(bounds + 2)) / 3),
		z = (int32_t)((*(bounds + 4) + *(bounds + 5)) / 2),
		c = phd_cos(item->pos.y_rot),
		s = phd_sin(item->pos.y_rot);

	target->x = item->pos.x_pos + ((c * x + s * z) >> W2V_SHIFT);
	target->y = item->pos.y_pos + y;
	target->z = item->pos.z_pos + ((c * z - s * x) >> W2V_SHIFT);
	target->room_number = item->room_number;
}

void AimWeapon(WEAPON_INFO* winfo, LARA_ARM* arm)
{
	PHD_ANGLE destx, desty;

	if (arm->lock)
	{
		desty = lara.target_angles[0];
		destx = lara.target_angles[1];
	}
	else desty = destx = 0;

	auto curr = arm->y_rot,
		 speed = winfo->aim_speed;

	if (curr >= desty - speed && curr <= desty + speed) curr = desty;
	else if (curr < desty)								curr += speed;
	else												curr -= speed;

	arm->y_rot = curr;

	curr = arm->x_rot;

	if (curr >= destx - speed && curr <= destx + speed) curr = destx;
	else if (curr < destx)								curr += speed;
	else												curr -= speed;

	arm->x_rot = curr;
	arm->z_rot = 0;
}

int FireWeapon(int weapon_type, ITEM_INFO* target, ITEM_INFO* src, PHD_ANGLE* angles)
{
	AMMO_INFO* ammo;

	switch (weapon_type)
	{
	case LG_MAGNUMS: ammo = &lara.magnums; break;
	case LG_UZIS:	 ammo = &lara.uzis; break;
	case LG_M16:	 ammo = &lara.m16; break;
	case LG_SHOTGUN: ammo = &lara.shotgun; break;
	default:		(ammo = &lara.pistols)->ammo = 1000; break;
	}

	if (ammo->ammo > 0)
	{
		--ammo->ammo;

		auto winfo = &weapons[weapon_type];

		PHD_3DPOS view { src->pos.x_pos, src->pos.y_pos - winfo->gun_height, src->pos.z_pos };

		view.x_rot = *(angles + 1) + (int16_t)((int)((GetRandomControl() - 16384) * winfo->shot_accuracy) / 65536);
		view.y_rot = *(angles + 0) + (int16_t)((int)((GetRandomControl() - 16384) * winfo->shot_accuracy) / 65536);
		view.z_rot = 0;

		phd_GenerateW2V(view);

		SPHERE slist[33];

		int nums = GetSpheres(target, slist, 0),
			best = -1,
			bestdist = 0x7fffffff;

		auto sptr = &slist[0];

		for (int i = 0; i < nums; ++i, ++sptr)
		{
			int r = sptr->r;

			if (ABS(sptr->x) < r &&
				ABS(sptr->y) < r &&
				sptr->z > r &&
				(sptr->x * sptr->x) + (sptr->y * sptr->y) <= (r * r))
			{
				if (sptr->z - r < bestdist)
				{
					bestdist = sptr->z - r;
					best = i;
				}
			}
		}

		lara.has_fired = 1;

		auto room_number = src->room_number;
		auto floor = GetFloor(view.x_pos, view.y_pos, view.z_pos, &room_number);

		GAME_VECTOR	vsrc { view.x_pos, view.y_pos, view.z_pos, room_number};

		if (best >= 0)
		{
			GAME_VECTOR	vdest
			{
				vsrc.x + ((*(phd_mxptr + M20) * bestdist) >> W2V_SHIFT),
				vsrc.y + ((*(phd_mxptr + M21) * bestdist) >> W2V_SHIFT),
				vsrc.z + ((*(phd_mxptr + M22) * bestdist) >> W2V_SHIFT)
			};

			int16_t smash_item = ObjectOnLOS(&vsrc, &vdest);

			if (smash_item != NO_ITEM && !(items[smash_item].object_number == SMASH_OBJECT1 && enable_smash1_destruction))
				SmashItem(smash_item, weapon_type);

			auto entity = g_level->get_entity_by_item(target);

			if (target->object_number != SHIVA && target->object_number != ARMY_WINSTON && target->object_number != LON_BOSS && (target->object_number != TRIBEBOSS || TribeBossShieldOn == 0))
				HitTarget(target, &vdest, winfo->damage, entity ? entity->get_type() == ENTITY_TYPE_PLAYER : false);
			else if (target->object_number == TRIBEBOSS)
			{
				int dx = (vdest.x - vsrc.x) >> 5,
					dy = (vdest.y - vsrc.y) >> 5,
					dz = (vdest.z - vsrc.z) >> 5;

				FindClosestShieldPoint(vdest.x - dx, vdest.y - dy, vdest.z - dz, target);
			}
			else if (target->object_number == ARMY_WINSTON || target->object_number == LON_BOSS)
			{
				target->hit_status = 1;
				--target->hit_points;

				TriggerRicochetSpark(&vdest, (m_atan2(lara_item->pos.z_pos, lara_item->pos.x_pos, target->pos.z_pos, target->pos.x_pos) >> 4) & 4095, 16);

				g_audio->play_sound(10, { target->pos.x_pos, target->pos.y_pos, target->pos.z_pos });
			}
			else
			{
				int z = target->pos.z_pos - lara_item->pos.z_pos,
					x = target->pos.x_pos - lara_item->pos.x_pos;

				int16_t angle = 0x8000 + phd_atan(z, x) - target->pos.y_rot;

				if ((target->current_anim_state > 1 && target->current_anim_state < 5) && angle < 0x4000 && angle > -0x4000)
				{
					target->hit_status = 1;

					TriggerRicochetSpark(&vdest, (m_atan2(lara_item->pos.z_pos, lara_item->pos.x_pos, target->pos.z_pos, target->pos.x_pos) >> 4) & 4095, 16);

					g_audio->play_sound(10, { target->pos.x_pos, target->pos.y_pos, target->pos.z_pos });
				}
				else HitTarget(target, &vdest, winfo->damage);
			}
			return 1;
		}
		else
		{
			GAME_VECTOR	vdest
			{
				vsrc.x + ((*(phd_mxptr + M20) * winfo->target_dist) >> W2V_SHIFT),
				vsrc.y + ((*(phd_mxptr + M21) * winfo->target_dist) >> W2V_SHIFT),
				vsrc.z + ((*(phd_mxptr + M22) * winfo->target_dist) >> W2V_SHIFT),
			};
			
			best = LOS(&vsrc, &vdest);

			int16_t smash_item = ObjectOnLOS(&vsrc, &vdest);

			if (smash_item != NO_ITEM)
			{
				if (!(items[smash_item].object_number == SMASH_OBJECT1 && !enable_smash1_destruction))
					SmashItem(smash_item, weapon_type);
				else Richochet(&vdest);
			}
			else if (!best)
				Richochet(&vdest);

			return -1;
		}
	}
	else return (ammo->ammo = 0);
}

bool HitTarget(ITEM_INFO* item, GAME_VECTOR* hitpos, int damage, bool is_player)
{
	if (is_player && !g_client->get_game_settings().friendly_fire)
		return false;

	gns::entity::hit_damage info;

	const bool hit_localplayer = (item == lara_item);

	info.blood.create = false;

	game_entity_base* entity = nullptr;

	if (!hit_localplayer)
	{
		item->hit_status = 1;

		if (item->data)
			((CREATURE_INFO*)item->data)->hurt_by_lara = 1;

		entity = g_level->get_entity_by_item(item);
	}
	else entity = game_level::LOCALPLAYER();

	if (entity)
	{
		damage *= is_player ? 50 : 1;

		info.sid = entity->get_sync_id();
	}

	auto old_hp = item->hit_points,
		 new_hp = int16_t(old_hp - damage);

	if (hitpos)
	{
		if (item->object_number != TRIBEBOSS &&
			item->object_number != ROBOT_SENTRY_GUN &&
			item->object_number != TARGETS)
		{
			info.blood =
			{
				.x = hitpos->x,
				.y = hitpos->y,
				.z = hitpos->z,
				.speed = item->speed,
				.angle = -int16_t(m_atan2(lara_item->pos.x_pos, lara_item->pos.z_pos, item->pos.x_pos, item->pos.z_pos)) + ONE_DEGREE * 90,
				.room = item->room_number,
				.create = true,
			};

			DoBloodSplat(info.blood.x, info.blood.y, info.blood.z, info.blood.speed, info.blood.angle, info.blood.room);
		}
		else if (item->object_number == ROBOT_SENTRY_GUN)
		{
			for (int i = 0; i < 3; ++i)
				TriggerRicochetSpark(hitpos, GetRandomControl() & 4095, 0);

			g_audio->play_sound(10, { item->pos.x_pos, item->pos.y_pos, item->pos.z_pos });
		}
	}

	if (entity)
	{
		info.attacker_sid = g_client->get_sync_id();
		info.damage = damage;

		switch (lara.gun_type)
		{
		case LG_UNARMED:
		case LG_SKIDOO:		info.weapon = 0; break;
		case LG_PISTOLS:	info.weapon = GUN_ITEM; break;
		case LG_MAGNUMS:	info.weapon = MAGNUM_ITEM; break;
		case LG_UZIS:		info.weapon = UZI_ITEM; break;
		case LG_SHOTGUN:	info.weapon = SHOTGUN_ITEM; break;
		case LG_M16:		info.weapon = M16_ITEM; break;
		case LG_ROCKET:		info.weapon = ROCKET_GUN_ITEM; break;
		case LG_GRENADE:	info.weapon = GRENADE_GUN_ITEM; break;
		case LG_HARPOON:	info.weapon = HARPOON_ITEM; break;
		case LG_FLARE:		info.weapon = FLAREBOX_ITEM; break;
		}

		g_client->send_packet(ID_ENTITY_HIT_DAMAGE, info);

		if (g_level->is_entity_streamed(entity))
			item->hit_points -= damage;
	}
	else item->hit_points -= damage;

	return (old_hp > 0 && new_hp <= 0);
}

void SmashItem(int16_t item_number, int weapon_type)
{
	auto item = &items[item_number];

	switch (item->object_number)
	{
	case SMASH_WINDOW:
	case SMASH_OBJECT1:
	case SMASH_OBJECT2:
	case SMASH_OBJECT3:
		SmashWindow(item_number);
		break;
	case CARCASS:
	case EXTRAFX6:
	{
		if (item->status != ACTIVE)
		{
			item->status = ACTIVE;

			AddActiveItem(item_number);
		}
	}
	}
}

int WeaponObject(int weapon_type)
{
	switch (weapon_type)
	{
	case LG_MAGNUMS: return MAGNUM;
	case LG_UZIS:	 return UZI;
	case LG_SHOTGUN: return SHOTGUN;
	case LG_ROCKET:  return ROCKET_GUN;
	case LG_GRENADE: return GRENADE_GUN;
	case LG_HARPOON: return HARPOON_GUN;
	case LG_M16:	 return M16;
	default:		 return PISTOLS;
	}
}

PHD_VECTOR get_gun_shell_pos(int bone, int weapon)
{
	PHD_VECTOR pos;

	if (bone == HAND_L)
	{
		switch (weapon)
		{
		case LG_PISTOLS: pos = { -12, (PISTOLS_YOFF >> 2) + 16, PISTOLS_ZOFF }; break;
		case LG_MAGNUMS: pos = { -16, MAGNUMS_YOFF >> 2, MAGNUMS_ZOFF }; break;
		case LG_UZIS:	 pos = { -16, UZIS_YOFF >> 2, UZIS_ZOFF }; break;
		}

		get_lara_bone_pos(lara_item, &pos, HAND_L);
	}
	else
	{
		switch (weapon)
		{
		case LG_PISTOLS: pos = { 8, (PISTOLS_YOFF >> 2) + 16, PISTOLS_ZOFF }; break;
		case LG_MAGNUMS: pos = { 16, MAGNUMS_YOFF >> 26, MAGNUMS_ZOFF }; break;
		case LG_UZIS:	 pos = { 8, UZIS_YOFF >> 2, UZIS_ZOFF }; break;
		case LG_M16:	 pos = { 16, (M16_YOFF >> 1) - 112, M16_ZOFF - 32 }; break;
		case LG_SHOTGUN: pos = { 16, SHOTGUN_YOFF >> 1, SHOTGUN_ZOFF }; break;
		}

		get_lara_bone_pos(lara_item, &pos, HAND_R);
	}

	return pos;
}

void draw_weapon_smoke(ITEM_INFO* item, int weapon, int cl, int cr)
{
	if (cl)
	{
		PHD_VECTOR pos;

		int hand = HAND_R;

		switch (weapon)
		{
		case LG_ROCKET:		pos = { 0, ROCKET_YOFF - 96, ROCKET_ZOFF };					break;
		case LG_GRENADE:	pos = { 0, GRENADE_YOFF, GRENADE_ZOFF };					break;
		case LG_SHOTGUN:	pos = { -16, SHOTGUN_YOFF, SHOTGUN_ZOFF };					break;
		case LG_M16:		pos = { 0, M16_YOFF, M16_ZOFF };							break;
		case LG_PISTOLS:	pos = { 4, PISTOLS_YOFF, PISTOLS_ZOFF };	hand = HAND_L;	break;
		case LG_MAGNUMS:	pos = { 16, MAGNUMS_YOFF, MAGNUMS_ZOFF };	hand = HAND_L;	break;
		case LG_UZIS:		pos = { 8, UZIS_YOFF, UZIS_ZOFF };			hand = HAND_L;	break;
		}

		get_lara_bone_pos(item, &pos, hand);

		TriggerGunSmoke(pos.x, pos.y, pos.z, 0, 0, 0, 0, weapon, cl);
	}

	if (cr)
	{
		PHD_VECTOR pos;

		switch (weapon)
		{
		case LG_PISTOLS:	pos = { -16, PISTOLS_YOFF, PISTOLS_ZOFF };	break;
		case LG_MAGNUMS:	pos = { -32, MAGNUMS_YOFF, MAGNUMS_ZOFF };	break;
		case LG_UZIS:		pos = { -16, UZIS_YOFF, UZIS_ZOFF };		break;
		}

		get_lara_bone_pos(item, &pos, HAND_R);

		TriggerGunSmoke(pos.x, pos.y, pos.z, 0, 0, 0, 0, weapon, cr);
	}
}

bool is_2guns(short weapon)
{
	switch (weapon)
	{
	case LG_PISTOLS:
	case LG_MAGNUMS:
	case LG_UZIS:		return true;
	}

	return false;
}

bool is_1gun(short weapon)
{
	switch (weapon)
	{
	case LG_SHOTGUN:
	case LG_M16:
	case LG_ROCKET: 
	case LG_GRENADE:
	case LG_HARPOON: return true;
	}

	return false;
}

void DoProperDetection(short item_number, long x, long y, long z, long xv, long yv, long zv)
{
	auto item = &items[item_number];
	auto room_number = item->room_number;
	auto floor = GetFloor(x, y, z, &room_number);

	int oldheight = GetHeight(floor, x, y, z),
		oldonobj = OnObject,
		oldtype = height_type;

	room_number = item->room_number;
	floor = GetFloor(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, &room_number);

	if (int height = GetHeight(floor, item->pos.x_pos, item->pos.y_pos, item->pos.z_pos); item->pos.y_pos >= height)
	{
		int bs = 0;

		if ((height_type == BIG_SLOPE || height_type == DIAGONAL) && oldheight < height)
		{
			int yang = int((unsigned short)item->pos.y_rot);

			if (tiltyoff < 0)
			{
				if (yang >= 0x8000)
					bs = 1;
			}
			else if (tiltyoff > 0 && yang <= 0x8000)
				bs = 1;

			if (tiltxoff < 0)
			{
				if (yang >= 0x4000 && yang <= 0xc000)
					bs = 1;
			}
			else if (tiltxoff > 0 && yang <= 0x4000 || yang >= 0xc000)
				bs = 1;
		}

		if (y > (height + 32) && bs == 0 &&
			(((x >> WALL_SHIFT) != (item->pos.x_pos >> WALL_SHIFT)) ||
			((z >> WALL_SHIFT) != (item->pos.z_pos >> WALL_SHIFT))))
		{
			int xs;

			if ((x & (~(WALL_L - 1))) != (item->pos.x_pos & (~(WALL_L - 1))) &&
				(z & (~(WALL_L - 1))) != (item->pos.z_pos & (~(WALL_L - 1))))
			{
				xs = (abs(x - item->pos.x_pos) < abs(z - item->pos.z_pos));
			}
			else xs = 1;

			if ((x & (~(WALL_L - 1))) != (item->pos.x_pos & (~(WALL_L - 1))) && xs)
				item->pos.y_rot = (xv <= 0 ? 0x4000 + (0xc000 - item->pos.y_rot) : 0xc000 + (0x4000 - item->pos.y_rot));
			else item->pos.y_rot = 0x8000 - item->pos.y_rot;

			item->speed >>= 1;
			item->pos.x_pos = x;
			item->pos.y_pos = y;
			item->pos.z_pos = z;
		}
		else if (height_type == BIG_SLOPE || height_type == DIAGONAL)
		{
			item->speed -= item->speed >> 2;

			if (tiltyoff < 0 && abs(tiltyoff) - abs(tiltxoff) >= 2)
			{
				if (((unsigned short)item->pos.y_rot) > 0x8000)
				{
					item->pos.y_rot = 0x4000 + (0xc000 - (unsigned short)item->pos.y_rot - 1);

					if (item->fallspeed > 0)
						item->fallspeed = -(item->fallspeed >> 1);
				}
				else
				{
					if (item->speed < 32)
					{
						item->speed -= tiltyoff << 1;

						if ((unsigned short)item->pos.y_rot > 0x4000 && (unsigned short)item->pos.y_rot < 0xc000)
						{
							item->pos.y_rot -= 4096;

							if ((unsigned short)item->pos.y_rot < 0x4000)
								item->pos.y_rot = 0x4000;
						}
						else if ((unsigned short)item->pos.y_rot < 0x4000)
						{
							item->pos.y_rot += 4096;

							if ((unsigned short)item->pos.y_rot > 0x4000)
								item->pos.y_rot = 0x4000;
						}
					}

					item->fallspeed = (item->fallspeed > 0 ? -(item->fallspeed >> 1) : 0);
				}
			}
			else if (tiltyoff > 0 && abs(tiltyoff) - abs(tiltxoff) >= 2)
			{
				if (((unsigned short)item->pos.y_rot) < 0x8000)
				{
					item->pos.y_rot = 0xc000 + (0x4000 - (unsigned short)item->pos.y_rot - 1);

					if (item->fallspeed > 0)
						item->fallspeed = -(item->fallspeed >> 1);
				}
				else
				{
					if (item->speed < 32)
					{
						item->speed += tiltyoff << 1;

						if ((unsigned short)item->pos.y_rot > 0xc000 || (unsigned short)item->pos.y_rot < 0x4000)
						{
							item->pos.y_rot -= 4096;

							if ((unsigned short)item->pos.y_rot < 0xc000)
								item->pos.y_rot = PHD_ANGLE(0xC000);
						}
						else if ((unsigned short)item->pos.y_rot < 0xc000)
						{
							item->pos.y_rot += 4096;

							if ((unsigned short)item->pos.y_rot > 0xc000)
								item->pos.y_rot = PHD_ANGLE(0xC000);
						}
					}

					item->fallspeed = (item->fallspeed > 0 ? -(item->fallspeed >> 1) : 0);
				}
			}
			else if (tiltxoff < 0 && abs(tiltxoff) - abs(tiltyoff) >= 2)
			{
				if (((unsigned short)item->pos.y_rot) > 0x4000 && ((unsigned short)item->pos.y_rot) < 0xc000)
				{
					item->pos.y_rot = (0x8000 - item->pos.y_rot - 1);

					if (item->fallspeed > 0)
						item->fallspeed = -(item->fallspeed >> 1);
				}
				else
				{
					if (item->speed < 32)
					{
						item->speed -= tiltxoff << 1;

						if ((unsigned short)item->pos.y_rot < 0x8000)
						{
							item->pos.y_rot -= 4096;

							if ((unsigned short)item->pos.y_rot > 0xf000)
								item->pos.y_rot = 0;
						}
						else if ((unsigned short)item->pos.y_rot >= 0x8000)
						{
							item->pos.y_rot += 4096;

							if ((unsigned short)item->pos.y_rot < 0x1000)
								item->pos.y_rot = 0;
						}
					}

					item->fallspeed = (item->fallspeed > 0 ? -(item->fallspeed >> 1) : 0);
				}
			}
			else if (tiltxoff > 0 && abs(tiltxoff) - abs(tiltyoff) >= 2)
			{
				if (((unsigned short)item->pos.y_rot) > 0xc000 || ((unsigned short)item->pos.y_rot) < 0x4000)
				{
					item->pos.y_rot = (0x8000 - item->pos.y_rot - 1);

					if (item->fallspeed > 0)
						item->fallspeed = -(item->fallspeed >> 1);
				}
				else
				{
					if (item->speed < 32)
					{
						item->speed += tiltxoff << 1;

						if ((unsigned short)item->pos.y_rot > 0x8000)
						{
							item->pos.y_rot -= 4096;

							if ((unsigned short)item->pos.y_rot < 0x8000)
								item->pos.y_rot = PHD_ANGLE(0x8000);
						}
						else if ((unsigned short)item->pos.y_rot < 0x8000)
						{
							item->pos.y_rot += 4096;

							if ((unsigned short)item->pos.y_rot > 0x8000)
								item->pos.y_rot = PHD_ANGLE(0x8000);
						}
					}

					item->fallspeed = (item->fallspeed > 0 ? -(item->fallspeed >> 1) : 0);
				}
			}
			else if (tiltyoff < 0 && tiltxoff < 0)
			{
				if (((unsigned short)item->pos.y_rot) > 0x6000 && ((unsigned short)item->pos.y_rot) < 0xe000)
				{
					item->pos.y_rot = 0x2000 + (0xa000 - (unsigned short)item->pos.y_rot - 1);

					if (item->fallspeed > 0)
						item->fallspeed = -(item->fallspeed >> 1);
				}
				else
				{
					if (item->speed < 32)
					{
						item->speed += -tiltyoff - tiltxoff;

						if ((unsigned short)item->pos.y_rot > 0x2000 && (unsigned short)item->pos.y_rot < 0xa000)
						{
							item->pos.y_rot -= 4096;

							if ((unsigned short)item->pos.y_rot < 0x2000)
								item->pos.y_rot = 0x2000;
						}
						else if ((unsigned short)item->pos.y_rot != 0x2000)
						{
							item->pos.y_rot += 4096;

							if ((unsigned short)item->pos.y_rot > 0x2000)
								item->pos.y_rot = 0x2000;
						}
					}

					item->fallspeed = (item->fallspeed > 0 ? -(item->fallspeed >> 1) : 0);
				}
			}
			else if (tiltyoff < 0 && tiltxoff > 0)
			{
				if (((unsigned short)item->pos.y_rot) > 0xa000 || ((unsigned short)item->pos.y_rot) < 0x2000)
				{
					item->pos.y_rot = 0x6000 + (0xe000 - (unsigned short)item->pos.y_rot - 1);

					if (item->fallspeed > 0)
						item->fallspeed = -(item->fallspeed >> 1);
				}
				else
				{
					if (item->speed < 32)
					{
						item->speed += (-tiltyoff) + tiltxoff;

						if ((unsigned short)item->pos.y_rot < 0xe000 && (unsigned short)item->pos.y_rot > 0x6000)
						{
							item->pos.y_rot -= 4096;

							if ((unsigned short)item->pos.y_rot < 0x6000)
								item->pos.y_rot = 0x6000;
						}
						else if ((unsigned short)item->pos.y_rot != 0x6000)
						{
							item->pos.y_rot += 4096;

							if ((unsigned short)item->pos.y_rot > 0x6000)
								item->pos.y_rot = 0x6000;
						}
					}

					item->fallspeed = (item->fallspeed > 0 ? -(item->fallspeed >> 1) : 0);
				}
			}
			else if (tiltyoff > 0 && tiltxoff > 0)
			{
				if (((unsigned short)item->pos.y_rot) > 0xe000 || ((unsigned short)item->pos.y_rot) < 0x6000)
				{
					item->pos.y_rot = 0xa000 + (0x2000 - (unsigned short)item->pos.y_rot - 1);

					if (item->fallspeed > 0)
						item->fallspeed = -(item->fallspeed >> 1);
				}
				else
				{
					if (item->speed < 32)
					{
						item->speed += tiltyoff + tiltxoff;

						if ((unsigned short)item->pos.y_rot < 0x2000 || (unsigned short)item->pos.y_rot > 0xa000)
						{
							item->pos.y_rot -= 4096;

							if ((unsigned short)item->pos.y_rot < 0xa000)
								item->pos.y_rot = PHD_ANGLE(0xa000);
						}
						else if ((unsigned short)item->pos.y_rot != 0xa000)
						{
							item->pos.y_rot += 4096;

							if ((unsigned short)item->pos.y_rot > 0xa000)
								item->pos.y_rot = PHD_ANGLE(0xa000);
						}
					}

					item->fallspeed = (item->fallspeed > 0 ? -(item->fallspeed >> 1) : 0);
				}
			}
			else if (tiltyoff > 0 && tiltxoff < 0)
			{
				if (((unsigned short)item->pos.y_rot) > 0x2000 && ((unsigned short)item->pos.y_rot) < 0xa000)
				{
					item->pos.y_rot = 0xe000 + (0x6000 - (unsigned short)item->pos.y_rot - 1);

					if (item->fallspeed > 0)
						item->fallspeed = -(item->fallspeed >> 1);
				}
				else
				{
					if (item->speed < 32)
					{
						item->speed += tiltyoff - tiltxoff;

						if ((unsigned short)item->pos.y_rot < 0x6000 || (unsigned short)item->pos.y_rot > 0xe000)
						{
							item->pos.y_rot -= 4096;

							if ((unsigned short)item->pos.y_rot < 0xe000)
								item->pos.y_rot = PHD_ANGLE(0xe000);
						}
						else if ((unsigned short)item->pos.y_rot != 0xe000)
						{
							item->pos.y_rot += 4096;

							if ((unsigned short)item->pos.y_rot > 0xe000)
								item->pos.y_rot = PHD_ANGLE(0xe000);
						}
					}

					item->fallspeed = (item->fallspeed > 0 ? -(item->fallspeed >> 1) : 0);
				}
			}

			item->pos.x_pos = x;
			item->pos.y_pos = y;
			item->pos.z_pos = z;
		}
		else
		{
			if (item->fallspeed > 0)
			{
				if (item->fallspeed > 16)
				{
					if (item->object_number == GRENADE)
						item->fallspeed = -(item->fallspeed - (item->fallspeed >> 1));
					else if ((item->fallspeed = -(item->fallspeed >> 2)) < -100)
						item->fallspeed = -100;
				}
				else
				{
					item->fallspeed = 0;

					if (item->object_number == GRENADE)
					{
						item->required_anim_state = 1;
						item->pos.x_rot = 0;
						--item->speed;
					}
					else item->speed -= 3;

					if (item->speed < 0)
						item->speed = 0;
				}
			}
			item->pos.y_pos = height;
		}
	}
	else
	{
		if (yv >= 0)
		{
			room_number = item->room_number;
			oldheight = GetHeight(GetFloor(item->pos.x_pos, y, item->pos.z_pos, &room_number), item->pos.x_pos, y, item->pos.z_pos);
			oldonobj = OnObject;
			room_number = item->room_number;

			GetHeight(GetFloor(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, &room_number), item->pos.x_pos, item->pos.y_pos, item->pos.z_pos);

			if (item->pos.y_pos >= oldheight && oldonobj)
			{
				if (item->fallspeed > 0)
				{
					if (item->fallspeed > 16)
					{
						if (item->object_number == GRENADE)							 item->fallspeed = -(item->fallspeed - (item->fallspeed >> 1));
						else if ((item->fallspeed = -(item->fallspeed >> 2)) < -100) item->fallspeed = -100;
					}
					else
					{
						item->fallspeed = 0;

						if (item->object_number == GRENADE)
						{
							item->required_anim_state = 1;
							item->pos.x_rot = 0;
							--item->speed;
						}
						else item->speed -= 3;

						if (item->speed < 0)
							item->speed = 0;
					}
				}

				item->pos.y_pos = oldheight;
			}
		}

		room_number = item->room_number;
		floor = GetFloor(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, &room_number);

		if (int ceiling = GetCeiling(floor, item->pos.x_pos, item->pos.y_pos, item->pos.z_pos); item->pos.y_pos < ceiling)
		{
			if (y < ceiling && ((x >> WALL_SHIFT) != (item->pos.x_pos >> WALL_SHIFT) || (z >> WALL_SHIFT) != (item->pos.z_pos >> WALL_SHIFT)))
			{
				if ((x & (~(WALL_L - 1))) != (item->pos.x_pos & (~(WALL_L - 1))))
					item->pos.y_rot = (xv <= 0 ? 0x4000 + (0xc000 - item->pos.y_rot) : 0xc000 + (0x4000 - item->pos.y_rot));
				else item->pos.y_rot = 0x8000 - item->pos.y_rot;

				if (item->object_number == GRENADE)
					item->speed -= item->speed >> 3;
				else item->speed >>= 1;

				item->pos.x_pos = x;
				item->pos.y_pos = y;
				item->pos.z_pos = z;
			}
			else item->pos.y_pos = ceiling;

			if (item->fallspeed < 0)
				item->fallspeed = -item->fallspeed;
		}
	}

	room_number = item->room_number;

	floor = GetFloor(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, &room_number);

	if (room_number != item->room_number)
		ItemNewRoom(item_number, room_number);
}

void DoProperDetectionFx(short fx_number, long x, long y, long z, long xv, long yv, long zv)
{
	auto item = &effects[fx_number];
	auto room_number = item->room_number;
	auto floor = GetFloor(x, y, z, &room_number);

	int oldheight = GetHeight(floor, x, y, z),
		oldonobj = OnObject,
		oldtype = height_type;

	room_number = item->room_number;
	floor = GetFloor(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, &room_number);

	if (int height = GetHeight(floor, item->pos.x_pos, item->pos.y_pos, item->pos.z_pos); item->pos.y_pos >= height)
	{
		int bs = 0;

		if ((height_type == BIG_SLOPE || height_type == DIAGONAL) && oldheight < height)
		{
			int yang = int((unsigned short)item->pos.y_rot);

			if (tiltyoff < 0)
			{
				if (yang >= 0x8000)
					bs = 1;
			}
			else if (tiltyoff > 0 && yang <= 0x8000)
				bs = 1;

			if (tiltxoff < 0)
			{
				if (yang >= 0x4000 && yang <= 0xc000)
					bs = 1;
			}
			else if (tiltxoff > 0 && yang <= 0x4000 || yang >= 0xc000)
				bs = 1;
		}

		if (y > (height + 32) && bs == 0 &&
			(((x >> WALL_SHIFT) != (item->pos.x_pos >> WALL_SHIFT)) ||
				((z >> WALL_SHIFT) != (item->pos.z_pos >> WALL_SHIFT))))
		{
			int xs;

			if ((x & (~(WALL_L - 1))) != (item->pos.x_pos & (~(WALL_L - 1))) &&
				(z & (~(WALL_L - 1))) != (item->pos.z_pos & (~(WALL_L - 1))))
			{
				xs = (abs(x - item->pos.x_pos) < abs(z - item->pos.z_pos));
			}
			else xs = 1;

			if ((x & (~(WALL_L - 1))) != (item->pos.x_pos & (~(WALL_L - 1))) && xs)
				item->pos.y_rot = (xv <= 0 ? 0x4000 + (0xc000 - item->pos.y_rot) : 0xc000 + (0x4000 - item->pos.y_rot));
			else item->pos.y_rot = 0x8000 - item->pos.y_rot;

			item->speed >>= 1;
			item->pos.x_pos = x;
			item->pos.y_pos = y;
			item->pos.z_pos = z;
		}
		else if (height_type == BIG_SLOPE || height_type == DIAGONAL)
		{
			item->speed -= item->speed >> 2;

			if (tiltyoff < 0 && abs(tiltyoff) - abs(tiltxoff) >= 2)
			{
				if (((unsigned short)item->pos.y_rot) > 0x8000)
				{
					item->pos.y_rot = 0x4000 + (0xc000 - (unsigned short)item->pos.y_rot - 1);

					if (item->fallspeed > 0)
						item->fallspeed = -(item->fallspeed >> 1);
				}
				else
				{
					if (item->speed < 32)
					{
						item->speed -= tiltyoff << 1;

						if ((unsigned short)item->pos.y_rot > 0x4000 && (unsigned short)item->pos.y_rot < 0xc000)
						{
							item->pos.y_rot -= 4096;

							if ((unsigned short)item->pos.y_rot < 0x4000)
								item->pos.y_rot = 0x4000;
						}
						else if ((unsigned short)item->pos.y_rot < 0x4000)
						{
							item->pos.y_rot += 4096;

							if ((unsigned short)item->pos.y_rot > 0x4000)
								item->pos.y_rot = 0x4000;
						}
					}

					item->fallspeed = (item->fallspeed > 0 ? -(item->fallspeed >> 1) : 0);
				}
			}
			else if (tiltyoff > 0 && abs(tiltyoff) - abs(tiltxoff) >= 2)
			{
				if (((unsigned short)item->pos.y_rot) < 0x8000)
				{
					item->pos.y_rot = 0xc000 + (0x4000 - (unsigned short)item->pos.y_rot - 1);

					if (item->fallspeed > 0)
						item->fallspeed = -(item->fallspeed >> 1);
				}
				else
				{
					if (item->speed < 32)
					{
						item->speed += tiltyoff << 1;

						if ((unsigned short)item->pos.y_rot > 0xc000 || (unsigned short)item->pos.y_rot < 0x4000)
						{
							item->pos.y_rot -= 4096;

							if ((unsigned short)item->pos.y_rot < 0xc000)
								item->pos.y_rot = PHD_ANGLE(0xC000);
						}
						else if ((unsigned short)item->pos.y_rot < 0xc000)
						{
							item->pos.y_rot += 4096;

							if ((unsigned short)item->pos.y_rot > 0xc000)
								item->pos.y_rot = PHD_ANGLE(0xC000);
						}
					}

					item->fallspeed = (item->fallspeed > 0 ? -(item->fallspeed >> 1) : 0);
				}
			}
			else if (tiltxoff < 0 && abs(tiltxoff) - abs(tiltyoff) >= 2)
			{
				if (((unsigned short)item->pos.y_rot) > 0x4000 && ((unsigned short)item->pos.y_rot) < 0xc000)
				{
					item->pos.y_rot = (0x8000 - item->pos.y_rot - 1);

					if (item->fallspeed > 0)
						item->fallspeed = -(item->fallspeed >> 1);
				}
				else
				{
					if (item->speed < 32)
					{
						item->speed -= tiltxoff << 1;

						if ((unsigned short)item->pos.y_rot < 0x8000)
						{
							item->pos.y_rot -= 4096;

							if ((unsigned short)item->pos.y_rot > 0xf000)
								item->pos.y_rot = 0;
						}
						else if ((unsigned short)item->pos.y_rot >= 0x8000)
						{
							item->pos.y_rot += 4096;

							if ((unsigned short)item->pos.y_rot < 0x1000)
								item->pos.y_rot = 0;
						}
					}

					item->fallspeed = (item->fallspeed > 0 ? -(item->fallspeed >> 1) : 0);
				}
			}
			else if (tiltxoff > 0 && abs(tiltxoff) - abs(tiltyoff) >= 2)
			{
				if (((unsigned short)item->pos.y_rot) > 0xc000 || ((unsigned short)item->pos.y_rot) < 0x4000)
				{
					item->pos.y_rot = (0x8000 - item->pos.y_rot - 1);

					if (item->fallspeed > 0)
						item->fallspeed = -(item->fallspeed >> 1);
				}
				else
				{
					if (item->speed < 32)
					{
						item->speed += tiltxoff << 1;

						if ((unsigned short)item->pos.y_rot > 0x8000)
						{
							item->pos.y_rot -= 4096;

							if ((unsigned short)item->pos.y_rot < 0x8000)
								item->pos.y_rot = PHD_ANGLE(0x8000);
						}
						else if ((unsigned short)item->pos.y_rot < 0x8000)
						{
							item->pos.y_rot += 4096;

							if ((unsigned short)item->pos.y_rot > 0x8000)
								item->pos.y_rot = PHD_ANGLE(0x8000);
						}
					}

					item->fallspeed = (item->fallspeed > 0 ? -(item->fallspeed >> 1) : 0);
				}
			}
			else if (tiltyoff < 0 && tiltxoff < 0)
			{
				if (((unsigned short)item->pos.y_rot) > 0x6000 && ((unsigned short)item->pos.y_rot) < 0xe000)
				{
					item->pos.y_rot = 0x2000 + (0xa000 - (unsigned short)item->pos.y_rot - 1);

					if (item->fallspeed > 0)
						item->fallspeed = -(item->fallspeed >> 1);
				}
				else
				{
					if (item->speed < 32)
					{
						item->speed += -tiltyoff - tiltxoff;

						if ((unsigned short)item->pos.y_rot > 0x2000 && (unsigned short)item->pos.y_rot < 0xa000)
						{
							item->pos.y_rot -= 4096;

							if ((unsigned short)item->pos.y_rot < 0x2000)
								item->pos.y_rot = 0x2000;
						}
						else if ((unsigned short)item->pos.y_rot != 0x2000)
						{
							item->pos.y_rot += 4096;

							if ((unsigned short)item->pos.y_rot > 0x2000)
								item->pos.y_rot = 0x2000;
						}
					}

					item->fallspeed = (item->fallspeed > 0 ? -(item->fallspeed >> 1) : 0);
				}
			}
			else if (tiltyoff < 0 && tiltxoff > 0)
			{
				if (((unsigned short)item->pos.y_rot) > 0xa000 || ((unsigned short)item->pos.y_rot) < 0x2000)
				{
					item->pos.y_rot = 0x6000 + (0xe000 - (unsigned short)item->pos.y_rot - 1);

					if (item->fallspeed > 0)
						item->fallspeed = -(item->fallspeed >> 1);
				}
				else
				{
					if (item->speed < 32)
					{
						item->speed += (-tiltyoff) + tiltxoff;

						if ((unsigned short)item->pos.y_rot < 0xe000 && (unsigned short)item->pos.y_rot > 0x6000)
						{
							item->pos.y_rot -= 4096;

							if ((unsigned short)item->pos.y_rot < 0x6000)
								item->pos.y_rot = 0x6000;
						}
						else if ((unsigned short)item->pos.y_rot != 0x6000)
						{
							item->pos.y_rot += 4096;

							if ((unsigned short)item->pos.y_rot > 0x6000)
								item->pos.y_rot = 0x6000;
						}
					}

					item->fallspeed = (item->fallspeed > 0 ? -(item->fallspeed >> 1) : 0);
				}
			}
			else if (tiltyoff > 0 && tiltxoff > 0)
			{
				if (((unsigned short)item->pos.y_rot) > 0xe000 || ((unsigned short)item->pos.y_rot) < 0x6000)
				{
					item->pos.y_rot = 0xa000 + (0x2000 - (unsigned short)item->pos.y_rot - 1);

					if (item->fallspeed > 0)
						item->fallspeed = -(item->fallspeed >> 1);
				}
				else
				{
					if (item->speed < 32)
					{
						item->speed += tiltyoff + tiltxoff;

						if ((unsigned short)item->pos.y_rot < 0x2000 || (unsigned short)item->pos.y_rot > 0xa000)
						{
							item->pos.y_rot -= 4096;

							if ((unsigned short)item->pos.y_rot < 0xa000)
								item->pos.y_rot = PHD_ANGLE(0xa000);
						}
						else if ((unsigned short)item->pos.y_rot != 0xa000)
						{
							item->pos.y_rot += 4096;

							if ((unsigned short)item->pos.y_rot > 0xa000)
								item->pos.y_rot = PHD_ANGLE(0xa000);
						}
					}

					item->fallspeed = (item->fallspeed > 0 ? -(item->fallspeed >> 1) : 0);
				}
			}
			else if (tiltyoff > 0 && tiltxoff < 0)
			{
				if (((unsigned short)item->pos.y_rot) > 0x2000 && ((unsigned short)item->pos.y_rot) < 0xa000)
				{
					item->pos.y_rot = 0xe000 + (0x6000 - (unsigned short)item->pos.y_rot - 1);

					if (item->fallspeed > 0)
						item->fallspeed = -(item->fallspeed >> 1);
				}
				else
				{
					if (item->speed < 32)
					{
						item->speed += tiltyoff - tiltxoff;

						if ((unsigned short)item->pos.y_rot < 0x6000 || (unsigned short)item->pos.y_rot > 0xe000)
						{
							item->pos.y_rot -= 4096;

							if ((unsigned short)item->pos.y_rot < 0xe000)
								item->pos.y_rot = PHD_ANGLE(0xe000);
						}
						else if ((unsigned short)item->pos.y_rot != 0xe000)
						{
							item->pos.y_rot += 4096;

							if ((unsigned short)item->pos.y_rot > 0xe000)
								item->pos.y_rot = PHD_ANGLE(0xe000);
						}
					}

					item->fallspeed = (item->fallspeed > 0 ? -(item->fallspeed >> 1) : 0);
				}
			}

			item->pos.x_pos = x;
			item->pos.y_pos = y;
			item->pos.z_pos = z;
		}
		else
		{
			if (item->fallspeed > 0)
			{
				if (item->fallspeed > 16)
				{
					if (item->object_number == GRENADE)
						item->fallspeed = -(item->fallspeed - (item->fallspeed >> 1));
					else if ((item->fallspeed = -(item->fallspeed >> 2)) < -100)
						item->fallspeed = -100;
				}
				else
				{
					item->fallspeed = 0;

					item->speed -= 3;

					if (item->speed < 0)
						item->speed = 0;
				}
			}
			item->pos.y_pos = height;
		}
	}
	else
	{
		if (yv >= 0)
		{
			room_number = item->room_number;
			oldheight = GetHeight(GetFloor(item->pos.x_pos, y, item->pos.z_pos, &room_number), item->pos.x_pos, y, item->pos.z_pos);
			oldonobj = OnObject;
			room_number = item->room_number;

			GetHeight(GetFloor(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, &room_number), item->pos.x_pos, item->pos.y_pos, item->pos.z_pos);

			if (item->pos.y_pos >= oldheight && oldonobj)
			{
				if (item->fallspeed > 0)
				{
					if (item->fallspeed > 16)
					{
						if (item->object_number == GRENADE)							 item->fallspeed = -(item->fallspeed - (item->fallspeed >> 1));
						else if ((item->fallspeed = -(item->fallspeed >> 2)) < -100) item->fallspeed = -100;
					}
					else
					{
						item->fallspeed = 0;

						item->speed -= 3;

						if (item->speed < 0)
							item->speed = 0;
					}
				}

				item->pos.y_pos = oldheight;
			}
		}

		room_number = item->room_number;
		floor = GetFloor(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, &room_number);

		if (int ceiling = GetCeiling(floor, item->pos.x_pos, item->pos.y_pos, item->pos.z_pos); item->pos.y_pos < ceiling)
		{
			if (y < ceiling && ((x >> WALL_SHIFT) != (item->pos.x_pos >> WALL_SHIFT) || (z >> WALL_SHIFT) != (item->pos.z_pos >> WALL_SHIFT)))
			{
				if ((x & (~(WALL_L - 1))) != (item->pos.x_pos & (~(WALL_L - 1))))
					item->pos.y_rot = (xv <= 0 ? 0x4000 + (0xc000 - item->pos.y_rot) : 0xc000 + (0x4000 - item->pos.y_rot));
				else item->pos.y_rot = 0x8000 - item->pos.y_rot;

				if (item->object_number == GRENADE)
					item->speed -= item->speed >> 3;
				else item->speed >>= 1;

				item->pos.x_pos = x;
				item->pos.y_pos = y;
				item->pos.z_pos = z;
			}
			else item->pos.y_pos = ceiling;

			if (item->fallspeed < 0)
				item->fallspeed = -item->fallspeed;
		}
	}

	room_number = item->room_number;

	floor = GetFloor(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, &room_number);

	if (room_number != item->room_number)
		EffectNewRoom(fx_number, room_number);
}